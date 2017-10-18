#include "stdafx.h"
#include "TaskSchedulerAPI.h"
#include "ComInitialize.h"

namespace task_scheduler {

	ComInitialize::ComInitialize() : initialized(false), initResult(S_OK)
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

	ComInitialize::~ComInitialize()
	{
		if (initialized) {
			CoUninitialize();
		}
	}

}