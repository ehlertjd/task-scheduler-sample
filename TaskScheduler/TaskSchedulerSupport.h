#pragma once

namespace task_scheduler {
	
	// Initializes the task service, and connects to the default folder
	// pTaskSvc and pTaskFolder will be initialized if this function returns S_OK
	HRESULT InitTaskServiceAndRootFolder(CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskFolder> &pTaskFolder);

	// Create a task with a trigger
	// pTask will be created if this function returns S_OK
	HRESULT CreateDailyTaskWithTrigger(const CComPtr<ITaskService> &pTaskSvc, CComPtr<ITaskDefinition> &pTask,
		const DateSpec &startDate, const DateSpec &endDate, const TimeSpec &dailyStartTime);

	// Create an executable action on the given task
	HRESULT CreateExecActionOnTask(const CComPtr<ITaskDefinition> &pTask, const wchar_t *taskExePath,
		const wchar_t **taskArgv, int32_t taskArgc);

	// Set the logon type for a task
	HRESULT SetTaskLogonType(const CComPtr<ITaskDefinition> &pTask, TASK_LOGON_TYPE logonType);

	// Set the task settings
	HRESULT SetTaskSettings(const CComPtr<ITaskDefinition> &pTask, bool startWhenAvailable);

	// Create a trigger for the task
	HRESULT SetTaskDailyTrigger(const CComPtr<ITaskDefinition> &pTask, const DateSpec &startDate,
		const DateSpec &endDate, const TimeSpec &dailyStartTime);

}