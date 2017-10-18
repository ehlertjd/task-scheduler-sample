// TaskSchedulerExe.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <string>
#include <ctime>

using namespace task_scheduler;
static const wchar_t TEST_TASK_NAME[] = L"TEST_TASK";
static const wchar_t TEST_EVENT_NAME[] = L"Global\\TASK_SCHEDULER_TEST_EVENT";

static int ScheduleTask(int argc, const wchar_t **argv);
static int DeleteTask(int argc, const wchar_t **argv);
static int RunTest(int argc, const wchar_t **argv);
static int SignalEvent();

static void PrintUsage(int argc, const wchar_t **argv) 
{
	const wchar_t *exeName = L"TaskScheduler";
	if (argc >= 1 && argv && argv[0]) {
		exeName = argv[0];
	}
	
	printf("Usage: %S <command> [options]\n", exeName);
	printf("Commands:\n");
	
	printf("\tschedule <name> - Schedule a new executable task to be run daily\n");
	printf("\t\t/ST <date> - The start date in the format of: YYYY/MM/DD\n");
	printf("\t\t/ET <date> - The optional end date in the format of: YYYY/MM/DD\n");
	printf("\t\t/T <time> - The time to run each day in the format of: HH:MM:SS (24-hour clock)\n");
	printf("\t\t/EXE <path> - The path to the executable\n\n");

	printf("\tdelete <name> - Delete a scheduled task, where <name> is the name of the task\n\n");

	printf("\ttest - Test scheduling a task and verifying execution\n\n");
}

int wmain(int argc, const wchar_t **argv) 
{
	// TODO: Parse args
	if (argc < 2) {
		PrintUsage(argc, argv);
		return 1;
	}

	std::wstring command(argv[1]);
	if (L"schedule" == command) {
		return ScheduleTask(argc, argv);
	} 
	if (L"delete" == command) {
		return DeleteTask(argc, argv);
	}
	if (L"test" == command) {
		return RunTest(argc, argv);
	}
	if (L"signal" == command) {
		return SignalEvent();
	}

	PrintUsage(argc, argv);
	return 1;
}

static int ScheduleTask(int argc, const wchar_t **argv) 
{
	// Parse arguments
	std::wstring taskName, startDateStr, endDateStr, timeStr, exePath;

	// Parse out arguments
	if (argc > 2) {
		taskName = argv[2];
	}

	for (int i = 3; i < (argc-1); i+=2) {
		std::wstring arg = argv[i];
		if (L"/ST" == arg) {
			startDateStr = argv[i + 1];
		} else if (L"/ET" == arg) {
			endDateStr = argv[i + 1];
		} else if (L"/T" == arg) {
			timeStr = argv[i + 1];
		} else if (L"/EXE" == arg) {
			exePath = argv[i + 1];
		} else {
			PrintUsage(argc, argv);
			printf("Unknown argument: %S\n", argv[i]);
			return 1;
		}
	}

	// Now validate arguments
	if (taskName.empty() || taskName[0] == L'/') {
		PrintUsage(argc, argv);
		printf("Invalid Task Name: %S\n", taskName.c_str());
		return 1;
	}

	if (exePath.empty()) {
		PrintUsage(argc, argv);
		printf("Executable path is required!\n");
		return 1;
	}

	DateSpec startDate, endDate;
	TimeSpec timeOfDay;

	if (!ParseDateString(startDate, startDateStr.c_str())) {
		PrintUsage(argc, argv);
		printf("Invalid start date, expected YYYY/MM/DD\n");
		return 1;
	}

	if (!ParseTimeString(timeOfDay, timeStr.c_str())) {
		PrintUsage(argc, argv);
		printf("Invalid time, expected HH:MM:SS\n");
		return 1;
	}

	ParseDateString(endDate, endDateStr.c_str());
	
	ScheduleTaskResult result = ScheduleDailyExecutableTask(taskName.c_str(), 
		startDate, endDate, timeOfDay, exePath.c_str(), NULL, 0);
	if (result == SCHEDULE_TASK_ERROR) {
		printf("Unable to schedule task!\n");
		return 10;
	}

	printf("The task %S has been created\n", taskName.c_str());
	return 0;
}

static int DeleteTask(int argc, const wchar_t **argv) 
{
	if (argc < 3) {
		// No task name
		PrintUsage(argc, argv);
		return 1;
	}

	if (DeleteTask(argv[2])) {
		printf("Deleted %S\n", argv[2]);
		return 0;
	} else {
		printf("Couldn't delete %S - maybe it doesn't exist?\n", argv[2]);
		return 10;
	}
}

static void CurrentTimeToTimeSpec(DateSpec &date, TimeSpec &time, int32_t addSeconds = 0) 
{
	// Get current time (adjusting as specified)
	time_t currentTime = ::time(NULL);
	currentTime += addSeconds;
	
	struct tm newtime;
	errno_t err = localtime_s(&newtime, &currentTime);
	if (err) {
		// Normally you'd want to throw or return a result, but we 
		// can't really recover in this context
		printf("Invalid argument to localtime_s\n");
		exit(10);
	}

	// Convert date spec
	date = DateSpec(newtime.tm_year + 1900, newtime.tm_mon, newtime.tm_mday - 1);
	// Convert time spec
	time = TimeSpec(newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
}

static int RunTest(int argc, const wchar_t **argv) 
{
	wchar_t exePath[MAX_PATH] = { 0 };
	static const wchar_t* TEST_ARGV[] = { L"signal" };
	static const int TEST_ARGC = sizeof(TEST_ARGV) / sizeof(wchar_t*);

	static_assert(1 == TEST_ARGC, "Expected TEST_ARGC to be 1!");

	bool success = false;
	DWORD dwRet = GetModuleFileName(NULL, exePath, MAX_PATH);
	if (0 == dwRet) {
		printf("Could not get executable name: %x\n", GetLastError());
		return 10;
	}

	// Get a time 3 seconds from now
	DateSpec date;
	TimeSpec time;
	CurrentTimeToTimeSpec(date, time, 3);

	// Create an inter-process event, so we can wake up as soon as the event fires
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, TEST_EVENT_NAME);
	if (hEvent == NULL) {
		printf("Could not create task event, exiting\n");
		return 10;
	}

	// Schedule the task
	ScheduleTaskResult result = task_scheduler::ScheduleDailyExecutableTask(
		TEST_TASK_NAME, date, DateSpec(), time, exePath, TEST_ARGV, TEST_ARGC);
	if (result != SCHEDULE_TASK_OK) {
		printf("Could not schedule the test task, exiting\n");
		CloseHandle(hEvent);
		return 10;
	}

	// Wait for up to 10 seconds for the task to run
	printf("Task was scheduled successfully! Waiting up to 10 seconds for execution...\n");
	DWORD dWaitResult = WaitForSingleObject(hEvent, 10000);
	if (dWaitResult == WAIT_OBJECT_0) {
		printf("Got a signal from the task process! TEST SUCCEEDED!\n");
		success = true;
	} else {
		printf("Timed out waiting for signal from task process. TEST FAILED!\n");
	}

	// Regardless, delete the task
	if (DeleteTask(TEST_TASK_NAME)) {
		printf("Successfully cleaned up the task.\n");
	} else {
		printf("Could not delete the task <%S>. You should manually delete this task!\n", TEST_TASK_NAME);
	}

	system("pause");
	return success ? 0 : 100;
}

static int SignalEvent() 
{
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEST_EVENT_NAME);
	if (hEvent == NULL) {
		printf("Could not open event for signaling: %x!\n", GetLastError());
		system("pause");
		return 10;
	}

	// Set the event and exit
	SetEvent(hEvent);
	CloseHandle(hEvent);
	return 0;
}

