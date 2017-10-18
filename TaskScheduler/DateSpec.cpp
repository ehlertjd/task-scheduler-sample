#include "stdafx.h"
#include "TaskSchedulerAPI.h"

namespace task_scheduler {

	DateSpec::DateSpec(uint16_t year, uint8_t month, uint8_t day): year(year), month(month), day(day)
	{ 
		// Normalize invalid values
		if (this->month > 11) {
			this->month = 0;
		}
		if (this->day > 30) {
			this->day = 0;
		}
	}

	uint16_t DateSpec::GetYear() const
	{
		return year;
	}

	uint8_t DateSpec::GetMonth() const
	{
		return month;
	}

	uint8_t DateSpec::GetDay() const
	{
		return day;
	}

	static __inline int32_t TimeSpecToSeconds(const TimeSpec &ts) {
		return (ts.GetHour() * 3600) + (ts.GetMinute() * 60) + ts.GetSecond();
	}

	TimeSpec::TimeSpec(uint8_t hour, uint8_t minute, uint8_t second): hour(hour), minute(minute), second(second)
	{
		if (this->hour > 23) {
			this->hour = 0;
		}
		if (this->minute > 59) {
			this->minute = 0;
		}
		if (this->second > 59) {
			this->second = 0;
		}
	}

	uint8_t TimeSpec::GetHour() const
	{
		return hour;
	}

	uint8_t TimeSpec::GetMinute() const
	{
		return minute;
	}

	uint8_t TimeSpec::GetSecond() const
	{
		return second;
	}

	int32_t TimeSpec::operator-(const TimeSpec &rhs) const
	{
		int32_t thisTime = TimeSpecToSeconds(*this);
		int32_t rhsTime = TimeSpecToSeconds(rhs);
		return thisTime - rhsTime;
	}

	TASKSCHEDULER_EXPORT bool FormatDateString(wchar_t *dst, size_t dstSize, const DateSpec &date, const TimeSpec &time) 
	{
		if (!dst || dstSize < DATE_FORMAT_STRING_SIZE) {
			return false;
		}
		
		int ret = _snwprintf_s(dst, dstSize, DATE_FORMAT_STRING_SIZE, L"%04hu-%02hhu-%02hhuT%02hhu:%02hhu:%02hhu",
			date.GetYear(), date.GetMonth()+1, date.GetDay()+1,
			time.GetHour(), time.GetMinute(), time.GetSecond());
		if (ret == -1) {
			return false;
		}

		return true;
	}

	TASKSCHEDULER_EXPORT bool ParseDateString(DateSpec &dst, const wchar_t *str)
	{
		uint16_t year;
		uint8_t month, day;
		if (!str) {
			return false;
		}
		int ret = swscanf_s(str, L"%hu/%hhu/%hhu", &year, &month, &day);
		if (ret != 3) {
			return false;
		}
		if (year < 1970) {
			return false;
		}

		dst = DateSpec(year, month-1, day-1);
		return true;
	}

	TASKSCHEDULER_EXPORT bool ParseTimeString(TimeSpec &dst, const wchar_t *str)
	{
		uint8_t hours, minutes, seconds;
		if (!str) {
			return false;
		}
		int ret = swscanf_s(str, L"%hhu:%hhu:%hhu", &hours, &minutes, &seconds);
		if (ret != 3) {
			return false;
		}
		dst = TimeSpec(hours, minutes, seconds);
		return true;
	}

}
