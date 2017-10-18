// TaskSchedulerExe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, const char *argv) {
	// TODO: Parse args
	task_scheduler::ScheduleTaskResult result = task_scheduler::ScheduleDailyExecutableTask(L"TestTask", 
		task_scheduler::DateSpec(2017, 10, 18), task_scheduler::DateSpec(), task_scheduler::TimeSpec(14), 
		L"C:\\Windows\\system32\\notepad.exe", NULL, 0);

	if (result == task_scheduler::SCHEDULE_TASK_OK) {
		return 0;
	}

    return 10;
}

