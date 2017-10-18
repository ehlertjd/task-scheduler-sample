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
			DateSpec spec(2017, 9, 3);
			wchar_t buffer[DATE_FORMAT_STRING_SIZE];

			FormatDateString(buffer, DATE_FORMAT_STRING_SIZE, spec);
			Assert::AreEqual(L"2017-10-04T00:00:00", buffer);
		}

		TEST_METHOD(ParseDateStringInvalidInput)
		{
			DateSpec spec;
			Assert::AreEqual(false, ParseDateString(spec, NULL));
			Assert::AreEqual(false, ParseDateString(spec, L""));
			Assert::AreEqual(false, ParseDateString(spec, L"2017"));
			Assert::AreEqual(false, ParseDateString(spec, L"2017/01"));
			Assert::AreEqual(false, ParseDateString(spec, L"1969/01/15"));

			Assert::AreEqual(0, (int)spec.GetYear());
			Assert::AreEqual(0, (int)spec.GetMonth());
			Assert::AreEqual(0, (int)spec.GetDay());
		}

		TEST_METHOD(ParseDateStringValidInput)
		{
			DateSpec spec;
			Assert::AreEqual(true, ParseDateString(spec, L"2017/01/15"));
			Assert::AreEqual(2017, (int)spec.GetYear());
			Assert::AreEqual(0, (int)spec.GetMonth());
			Assert::AreEqual(14, (int)spec.GetDay());
		}
	};
}