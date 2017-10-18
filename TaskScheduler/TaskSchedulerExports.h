#pragma once

// Define a macro for exporting classes / api methods
#ifdef TASKSCHEDULER_EXPORTS
  #define TASKSCHEDULER_EXPORT __declspec(dllexport)
#else
  #define TASKSCHEDULER_EXPORT __declspec(dllimport)
#endif

