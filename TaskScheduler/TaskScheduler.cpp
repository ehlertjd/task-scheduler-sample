// TaskScheduler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include "TaskSchedulerAPI.h"

#include "ComInitialize.h"
#include "TaskSchedulerSupport.h"

// A few notes about this module:
// Simplifying assumptions were made for the purposes of this exercise, it shouldn't be 
// difficult to make this more extensible and allow for detailed scheduling control.

// We assume that we want to schedule the task to be run as the current user, and that the user will be logged on,
// so we use the INTERACTIVE_TOKEN logon type for the task

namespace task_scheduler {

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

}