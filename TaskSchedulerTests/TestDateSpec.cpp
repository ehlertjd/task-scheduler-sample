#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task_scheduler;

namespace TaskSchedulerTests
{		
	TEST_CLASS(TestDateSpec)
	{
	public:
		
		TEST_METHOD(DefaultConstructor)
		{
			DateSpec spec;
			Assert::AreEqual(0, (int)spec.GetYear());
			Assert::AreEqual(0, (int)spec.GetMonth());
			Assert::AreEqual(0, (int)spec.GetDay());
		}

		TEST_METHOD(ConstructorValidValues)
		{
			DateSpec spec(2017, 8, 17);
			Assert::AreEqual(2017, (int)spec.GetYear());
			Assert::AreEqual(8, (int)spec.GetMonth());
			Assert::AreEqual(17, (int)spec.GetDay());
		}

		TEST_METHOD(ConstructorInvalidValues)
		{
			DateSpec spec(2017, 12, 31);
			Assert::AreEqual(2017, (int)spec.GetYear());
			Assert::AreEqual(0, (int)spec.GetMonth());
			Assert::AreEqual(0, (int)spec.GetDay());
		}

		TEST_METHOD(FormatDateStringDateOnly)
		{
			DateSpec spec(2017, 10, 4);
			wchar_t buffer[DATE_FORMAT_STRING_SIZE];

			FormatDateString(buffer, DATE_FORMAT_STRING_SIZE, spec);
			Assert::AreEqual(L"2017-10-04T00:00:00", buffer);
		}
	};
}