#pragma once

namespace task_scheduler {

	// Provides RAII COM Initialization and Uninitialization
	// Normally you would probably want to do this at application start / end
	class ComInitialize
	{
	public:
		bool initialized;
		HRESULT initResult;

		ComInitialize();
		~ComInitialize();
	};

}
