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

		TEST_METHOD(Subtraction) {
			TimeSpec spec1(8, 23, 15);
			TimeSpec spec2(6, 07, 10);

			Assert::AreEqual(8165, spec1 - spec2);
			Assert::AreEqual(-8165, spec2 - spec1);
		}


		TEST_METHOD(FormatDateStringDateAndTime)
		{
			DateSpec date(2018, 1, 14);
			TimeSpec time(13, 5, 33);
			wchar_t buffer[DATE_FORMAT_STRING_SIZE];

			FormatDateString(buffer, DATE_FORMAT_STRING_SIZE, date, time);
			Assert::AreEqual(L"2018-01-14T13:05:33", buffer);
		}

	};
}