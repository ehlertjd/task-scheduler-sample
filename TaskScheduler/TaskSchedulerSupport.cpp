#include "stdafx.h"
#include "TaskSchedulerAPI.h"
#include "TaskSchedulerSupport.h"

namespace task_scheduler {

	static const wchar_t * DEFAULT_TASK_FOLDER = L"\\";

	HRESULT InitTaskServiceAndRootFolder(CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskFolder> &pTaskFolder)
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

	HRESULT CreateDailyTaskWithTrigger(const CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskDefinition> &pTask,
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

	HRESULT CreateExecActionOnTask(const CComPtr<ITaskDefinition> &pTask, const wchar_t *taskExePath,
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


	HRESULT SetTaskLogonType(const CComPtr<ITaskDefinition> &pTask, TASK_LOGON_TYPE logonType)
	{
		CComPtr<IPrincipal> pPrincipal;
		HRESULT hr = pTask->get_Principal(&pPrincipal);
		if (FAILED(hr)) {
			return hr;
		}

		return pPrincipal->put_LogonType(logonType);
	}

	HRESULT SetTaskSettings(const CComPtr<ITaskDefinition> &pTask, bool startWhenAvailable)
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

	HRESULT SetTaskDailyTrigger(const CComPtr<ITaskDefinition> &pTask, const DateSpec &startDate,
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