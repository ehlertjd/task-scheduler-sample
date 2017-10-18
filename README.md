# Task Scheduler Sample

This is a Visual Studio 2015 project that provides a sample implementation of scheduling a task on windows using the TaskScheduler 2.0 interface.

There are 3 projects included in this solution:

### TaskScheduler
This is a DLL Project that provides a simple interface for scheduling an executable task for daily execution. 
It is structured such that it would be relatively easy to extend the library to allow the API user more control over the task.

### TaskSchedulerExe
This is a command-line executable project that makes use of the TaskScheduler.dll. 
It provides commands for scheduling and deleting tasks, as well as integration testing the task scheduler.
Executable usage is as follows:

```
Usage: TaskScheduler.exe <command> [options]
Commands:
        schedule <name> - Schedule a new executable task to be run daily
                /ST <date> - The start date in the format of: YYYY/MM/DD
                /ET <date> - The optional end date in the format of: YYYY/MM/DD
                /T <time> - The time to run each day in the format of: HH:MM:SS (24-hour clock)
                /EXE <path> - The path to the executable

        delete <name> - Delete a scheduled task, where <name> is the name of the task

        test - Test scheduling a task and verifying execution
```

### TaskSchedulerTests
This is a UnitTesting project that tests some of the exported APIs from the TaskScheduler.dll project.


