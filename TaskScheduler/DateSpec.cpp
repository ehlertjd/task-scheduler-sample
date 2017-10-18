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

}
