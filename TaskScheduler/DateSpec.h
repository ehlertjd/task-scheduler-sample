#pragma once
namespace task_scheduler {


	/**
	 * Allows specification of a specific date
	 */
	class TASKSCHEDULER_EXPORT DateSpec
	{
		/**
		 * The four digit year (e.g. 2017)
		 */
		uint16_t year;

		/**
		 * The month, starting from 0 (i.e. 0 = January)
		 */
		uint8_t month;

		/**
		 * The day of the month, starting from 0
		 */
		uint8_t day;

	public:
		/**
		 * Create a new date spec.
		 * @param year The four digit year
		 * @param month The month, starting from 0
		 * @param day The day of the month, starting from 0
		 * NOTE: Invalid values will default to 0.
		 */
		DateSpec(uint16_t year = 0, uint8_t month = 0, uint8_t day = 0);

		/**
		 * Get the year
		 */
		uint16_t GetYear() const;

		/**
		 * Get the month
		 */
		uint8_t GetMonth() const;

		/**
		 * Get the day
		 */
		uint8_t GetDay() const;
	};

	/**
	 * Allows specification of a time of day
	 */
	class TASKSCHEDULER_EXPORT TimeSpec
	{
	private:
		/**
		 * The hour, from 0-23
		 */
		uint8_t hour;

		/**
		 * The minute, from 0-59
		 */
		uint8_t minute;

		/**
		 * The second, from 0-59
		 */
		uint8_t second;

	public:
		/**
		 * Create a time spec.
		 * @param hour The hour of the day (0-23)
		 * @param minute The minute of the hour (0-59)
		 * @param second The second of the minute (0-59)
		 * NOTE: Invalid values will default to 0
		 */
		TimeSpec(uint8_t hour = 0, uint8_t minute = 0, uint8_t second = 0);

		/**
		* Get the hour specified
		*/
		uint8_t GetHour() const;

		/**
		* Get the minute specified
		*/
		uint8_t GetMinute() const;

		/**
		* Get the second specified
		*/
		uint8_t GetSecond() const;

		/**
		 * Subtract two timespecs to get a duration in seconds.
		 */
		int32_t operator-(const TimeSpec &rhs) const;
	};

}
