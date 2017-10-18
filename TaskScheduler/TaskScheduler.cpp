// TaskScheduler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include "TaskSchedulerAPI.h"

// A few notes about this module:
// Simplifying assumptions were made for the purposes of this exercise, it shouldn't be 
// difficult to make this more extensible and allow for detailed scheduling control.

// We assume that we want to schedule the task to be run as the current user, and that the user will be logged on,
// so we use the INTERACTIVE_TOKEN logon type for the task

static const wchar_t * DEFAULT_TASK_FOLDER = L"\\";

namespace task_scheduler {

	// Provides RAII COM Initialization
	class ComInitialize
	{
	public:
		bool initialized;
		HRESULT initResult;

		ComInitialize() : initialized(false), initResult(S_OK)
		{
			// Initialize COM & Set security levels
			initResult = CoInitializeEx(NULL, COINITBASE_MULTITHREADED);
			if (FAILED(initResult)) {
				printf("Unable to initialize COM: %x\n", initResult);
				return;
			}

			initialized = true;
			initResult = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
				RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
			if (FAILED(initResult)) {
				printf("Unable to initialize security: %x\n", initResult);
			}
		}

		~ComInitialize()
		{
			if (initialized) {
				CoUninitialize();
			}
		}
	};

	// Initializes the task service, and connects to the default folder
	// pTaskSvc and pTaskFolder will be initialized if this function returns S_OK
	static HRESULT InitTaskServiceAndRootFolder(CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskFolder> &pTaskFolder);
	
	// Create a task with a trigger
	// pTask will be created if this function returns S_OK
	static HRESULT CreateDailyTaskWithTrigger(const CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskDefinition> &pTask,
		const DateSpec &startDate, const DateSpec &endDate, const TimeSpec &dailyStartTime);

	// Create an executable action on the given task
	static HRESULT CreateExecActionOnTask(const CComPtr<ITaskDefinition> &pTask, const wchar_t *taskExePath,
		const wchar_t **taskArgv, int32_t taskArgc);

	TASKSCHEDULER_EXPORT ScheduleTaskResult ScheduleDailyExecutableTask(
		const wchar_t *taskName,
		const DateSpec &startDate,
		const DateSpec &endDate,
		const TimeSpec &dailyStartTime,
		const wchar_t *taskExePath,
		const wchar_t **taskArgv,
		int32_t taskArgc)
	{
		// Initialize COM & Set security levels
		// This will automatically uninitialize when we exit the function
		ComInitialize comInit;
		if (FAILED(comInit.initResult)) {
			return SCHEDULE_TASK_ERROR;
		}

		// Init service & root folder
		CComPtr<ITaskService> pTaskSvc;
		CComPtr<ITaskFolder> pTaskFolder;
		HRESULT hr = InitTaskServiceAndRootFolder(pTaskSvc, pTaskFolder);
		if (FAILED(hr)) {
			return SCHEDULE_TASK_ERROR;
		}

		// Delete the existing task, if it exists
		pTaskFolder->DeleteTask(_bstr_t(taskName), 0);

		// Create and configure the task to run daily
		CComPtr<ITaskDefinition> pTask;
		hr = CreateDailyTaskWithTrigger(pTaskSvc, pTask, startDate, endDate, dailyStartTime);
		if (FAILED(hr)) {
			printf("Could not create a new task: %x\n", hr);
			return SCHEDULE_TASK_ERROR;
		}

		// Once we create the task, the next step is to add an action
		// We'll add an executable action in this code path
		hr = CreateExecActionOnTask(pTask, taskExePath, taskArgv, taskArgc);
		if (FAILED(hr)) {
			printf("Could not create exec action on task: %x\n", hr);
			return SCHEDULE_TASK_ERROR;
		}
		
		// Finally, the last step is to register the task
		// Use current user - otherwise we'd have to supply username, password
		CComPtr<IRegisteredTask> pRegisteredTask;
		hr = pTaskFolder->RegisterTaskDefinition(_bstr_t(taskName), pTask, TASK_CREATE_OR_UPDATE,
			_variant_t(), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &pRegisteredTask);

		if (FAILED(hr)) {
			printf("Failed to register task: %x\n", hr);
			return SCHEDULE_TASK_ERROR;
		}

		return SCHEDULE_TASK_OK;
	}

	TASKSCHEDULER_EXPORT bool DeleteTask(const wchar_t *taskName) {
		// Initialize COM & Set security levels
		// This will automatically uninitialize when we exit the function
		ComInitialize comInit;
		if (FAILED(comInit.initResult)) {
			return false;
		}

		CComPtr<ITaskService> pTaskSvc;
		CComPtr<ITaskFolder> pTaskFolder;

		// Init service & root folder
		HRESULT hr = InitTaskServiceAndRootFolder(pTaskSvc, pTaskFolder);
		if (FAILED(hr)) {
			return false;
		}

		hr = pTaskFolder->DeleteTask(_bstr_t(taskName), 0);
		if (FAILED(hr)) {
			printf("Error deleting task: %x\n", hr);
			return false;
		}

		return true;
	}

	TASKSCHEDULER_EXPORT bool TaskExists(const wchar_t *taskName) {
		// Initialize COM & Set security levels
		// This will automatically uninitialize when we exit the function
		ComInitialize comInit;
		if (FAILED(comInit.initResult)) {
			return false;
		}

		CComPtr<ITaskService> pTaskSvc;
		CComPtr<ITaskFolder> pTaskFolder;

		// Init service & root folder
		HRESULT hr = InitTaskServiceAndRootFolder(pTaskSvc, pTaskFolder);
		if (FAILED(hr)) {
			return false;
		}

		CComPtr<IRegisteredTask> pTask;
		hr = pTaskFolder->GetTask(_bstr_t(taskName), &pTask);
		if (FAILED(hr)) {
			return false;
		}

		return !!pTask;
	}

	// ===== Helper Functions =====
	static __inline HRESULT InitTaskServiceAndRootFolder(CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskFolder> &pTaskFolder)
	{
		// Create an instance of the tasks scheduler, and connect
		HRESULT hr = pTaskSvc.CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER);
		if (FAILED(hr)) {
			printf("Unable to initialize task scheduler: %xn", hr);
			return hr;
		}

		// Connect locally
		hr = pTaskSvc->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if ((FAILED(hr))) {
			printf("Unable to connect task scheduler: %xn", hr);
			return hr;
		}

		// Get the folder that we want to schedule the task under:
		hr = pTaskSvc->GetFolder(_bstr_t(DEFAULT_TASK_FOLDER), &pTaskFolder);
		if (FAILED(hr)) {
			printf("Unable to open the default task folder <%S>: %x\n", DEFAULT_TASK_FOLDER, hr);
			return hr;
		}

		return S_OK;
	}

	// Set the logon type for a task
	static HRESULT SetTaskLogonType(const CComPtr<ITaskDefinition> &pTask, TASK_LOGON_TYPE logonType);
	// Set the task settings
	static HRESULT SetTaskSettings(const CComPtr<ITaskDefinition> &pTask, bool startWhenAvailable);
	// Create a trigger for the task
	static HRESULT SetTaskDailyTrigger(const CComPtr<ITaskDefinition> &pTask, const DateSpec &startDate,
		const DateSpec &endDate, const TimeSpec &dailyStartTime);

	static HRESULT CreateDailyTaskWithTrigger(const CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskDefinition> &pTask,
			const DateSpec &startDate, const DateSpec &endDate, const TimeSpec &dailyStartTime)
	{
		// Create and configure the task
		HRESULT hr = pTaskSvc->NewTask(0, &pTask);
		if (FAILED(hr)) {
			printf("Could not create a new task instance: %x\n", hr);
			return hr;
		}

		hr = SetTaskLogonType(pTask, TASK_LOGON_INTERACTIVE_TOKEN);
		if (FAILED(hr)) {
			printf("Could not set task logon type: %x\n", hr);
			return hr;
		}

		hr = SetTaskSettings(pTask, true);
		if (FAILED(hr)) {
			printf("Could not update task settings: %x\n", hr);
			return hr;
		}

		hr = SetTaskDailyTrigger(pTask, startDate, endDate, dailyStartTime);
		if (FAILED(hr)) {
			printf("Could not create task trigger: %x\n", hr);
			return hr;
		}

		return S_OK;
	}

	static HRESULT CreateExecActionOnTask(const CComPtr<ITaskDefinition> &pTask, const wchar_t *taskExePath,
			const wchar_t **taskArgv, int32_t taskArgc)
	{
		CComPtr<IActionCollection> pActions;
		HRESULT hr = pTask->get_Actions(&pActions);
		if (FAILED(hr)) {
			printf("Could not get action collection for task: %x\n", hr);
			return hr;
		}

		// Create the action
		CComPtr<IAction> pAction;
		hr = pActions->Create(TASK_ACTION_EXEC, &pAction);
		if (FAILED(hr)) {
			printf("Could not create an executable action for task: %x\n", hr);
			return hr;
		}

		// And query for the ExeAction interface
		CComPtr<IExecAction> pExeAction;
		hr = pAction.QueryInterface(&pExeAction);
		if (FAILED(hr)) {
			printf("Could not query for executable action: %x\n", hr);
			return hr;
		}
		pAction.Release();

		// TODO: Do we need to take a working directory here?
		hr = pExeAction->put_Path(_bstr_t(taskExePath));
		if (FAILED(hr)) {
			printf("Could not set executable action path: %x\n", hr);
			return hr;
		}

		if (taskArgv && taskArgc) {
			std::wstring args;
			// Known limitiation - we do not support arguments with spaces, it's up to the caller
			// To wrap their arguments in quotes and escape any necessary values
			for (int32_t i = 0; i < taskArgc; i++) {
				if (taskArgv[i] && taskArgv[i][0]) {
					if (!args.empty()) {
						args.push_back(L' ');
					}
					args.append(taskArgv[i]);
				}
			}

			hr = pExeAction->put_Arguments(_bstr_t(args.c_str()));
			if (FAILED(hr)) {
				printf("Could not set executable action arguments: %x\n", hr);
				return hr;
			}
		}

		return S_OK;
	}


	static HRESULT SetTaskLogonType(const CComPtr<ITaskDefinition> &pTask, TASK_LOGON_TYPE logonType)
	{
		CComPtr<IPrincipal> pPrincipal;
		HRESULT hr = pTask->get_Principal(&pPrincipal);
		if (FAILED(hr)) {
			return hr;
		}

		return pPrincipal->put_LogonType(logonType);
	}

	static HRESULT SetTaskSettings(const CComPtr<ITaskDefinition> &pTask, bool startWhenAvailable)
	{
		// Default
		if (!startWhenAvailable) {
			return S_OK;
		}

		// TODO: Add more setting options, such as idle time, etc.
		CComPtr<ITaskSettings> pSettings;
		HRESULT hr = pTask->get_Settings(&pSettings);
		if (FAILED(hr)) {
			return hr;
		}

		return pSettings->put_StartWhenAvailable(VARIANT_TRUE);
	}

	static HRESULT SetTaskDailyTrigger(const CComPtr<ITaskDefinition> &pTask, const DateSpec &startDate,
		const DateSpec &endDate, const TimeSpec &dailyStartTime)
	{
		CComPtr<ITriggerCollection> pTriggerCollection;
		CComPtr<ITrigger> pTrigger;
		CComPtr<IDailyTrigger> pDailyTrigger;
		wchar_t timespec[DATE_FORMAT_STRING_SIZE];

		HRESULT hr = pTask->get_Triggers(&pTriggerCollection);
		if (FAILED(hr)) {
			printf("Could not get trigger collection: %x\n", hr);
			return hr;
		}

		hr = pTriggerCollection->Create(TASK_TRIGGER_DAILY, &pTrigger);
		if (FAILED(hr)) {
			printf("Could not create daily trigger: %x\n", hr);
			return hr;
		}

		hr = pTrigger.QueryInterface(&pDailyTrigger);
		if (FAILED(hr)) {
			printf("Could not query daily trigger interface: %x\n", hr);
			return hr;
		}
		pTrigger.Release();
		
		// Set trigger id
		hr = pDailyTrigger->put_Id(_bstr_t("Daily Trigger"));
		if (FAILED(hr)) {
			printf("Could not set trigger id: %x\n", hr);
		}

		// Set start time
		FormatDateString(timespec, DATE_FORMAT_STRING_SIZE, startDate, dailyStartTime);
		hr = pDailyTrigger->put_StartBoundary(_bstr_t(timespec));
		if (FAILED(hr)) {
			printf("Could not set start time: %x\n", hr);
			return hr;
		}

		// Set end time (optional)
		if (endDate.GetYear()) {
			FormatDateString(timespec, DATE_FORMAT_STRING_SIZE, endDate);
			hr = pDailyTrigger->put_EndBoundary(_bstr_t(timespec));
			if (FAILED(hr)) {
				printf("Could not set end time: %x\n", hr);
				return hr;
			}
		}

		return S_OK;
	}
}