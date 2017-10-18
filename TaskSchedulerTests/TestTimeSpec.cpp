#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task_scheduler;

namespace TaskSchedulerTests
{
	TEST_CLASS(TestTimeSpec)
	{
	public:

		TEST_METHOD(DefaultConstructor)
		{
			TimeSpec spec;
			Assert::AreEqual(0, (int)spec.GetHour());
			Assert::AreEqual(0, (int)spec.GetMinute());
			Assert::AreEqual(0, (int)spec.GetSecond());
		}

		TEST_METHOD(ConstructorValidValues)
		{
			TimeSpec spec(8, 23, 15);
			Assert::AreEqual(8, (int)spec.GetHour());
			Assert::AreEqual(23, (int)spec.GetMinute());
			Assert::AreEqual(15, (int)spec.GetSecond());
		}

		TEST_METHOD(ConstructorInvalidValues)
		{
			TimeSpec spec(24, 60, 60);
			Assert::AreEqual(0, (int)spec.GetHour());
			Assert::AreEqual(0, (int)spec.GetMinute());
			Assert::AreEqual(0, (int)spec.GetSecond());
		}

	};
}