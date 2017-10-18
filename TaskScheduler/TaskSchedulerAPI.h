#pragma once

#include <atlbase.h>
#include <atlstr.h>
#include <cstdint>

#include "TaskSchedulerExports.h"

#include "DateSpec.h"

namespace task_scheduler {
	enum ScheduleTaskResult {
		SCHEDULE_TASK_OK,
		SCHEDULE_TASK_ERROR // Unspecified error
	};

	/**
	 * Schedules a task to be run once daily at the specified time, as the current user.
	 * @param startDate The starting date
	 * @param endDate The optional ending date
	 * @param dailyStartTime The time to start the task each day
	 * @param taskExePath The path to the executable
	 * @param taskArgv Optional task arguments
	 * @param taskArgc The number of arguments in the argv array
	 * @returns A result code indicating success or failure.
	 */
	TASKSCHEDULER_EXPORT ScheduleTaskResult ScheduleDailyExecutableTask(
		const wchar_t *taskName,
		const DateSpec &startDate,
		const DateSpec &endDate,
		const TimeSpec &dailyStartTime,
		const wchar_t *taskExePath,
		const wchar_t **taskArgv,
		int32_t taskArgc
	);

	/**
	 * Delete an existing task. If the specified task does not exist, this is a no-op.
	 * @param taskName The name of the task to delete.
	 * @returns true if the operation succeeded, false otherwise
	 */
	TASKSCHEDULER_EXPORT bool DeleteTask(const wchar_t *taskName);

}

