#ifndef DATE_UTILS_HEADER
#define DATE_UTILS_HEADER

#include <stdbool.h>

/**
 * A calendar date encoded as a single comparable integer in YYYYMMDD form
 * (e.g., "31-12-2026" -> 20261231). Ordering of these integers matches the
 * chronological ordering of the dates they represent, so a range check is just
 * an integer comparison.
 */
typedef int DateValue;

/**
 * Sentinel returned when a date cannot be parsed or does not correspond to a
 * real calendar date. It is intentionally out of the valid YYYYMMDD range.
 */
#define INVALID_DATE_VALUE (-1)

/**
 * Bytes required to hold an ISO "YYYY-MM-DD" date literal: 10 characters plus
 * the null terminator. Every buffer passed to formatDateValueIso must be at
 * least this size.
 */
#define ISO_DATE_BUFFER_SIZE 11

/** Returns true if the given year is a leap year in the Gregorian calendar. */
bool isLeapYear(const int year);

/**
 * Returns the number of days in the given month (1-12) for the given year,
 * accounting for leap years on February. Returns 0 if the month is out of
 * range.
 */
int daysInMonth(const int month, const int year);

/**
 * Returns true if (day, month, year) is a real calendar date: month in 1-12 and
 * day within the valid range for that month and year (leap years included).
 */
bool isValidCalendarDate(const int day, const int month, const int year);

/**
 * Parses a "DD-MM-YYYY" date literal into a comparable YYYYMMDD integer.
 * Returns INVALID_DATE_VALUE if the string is malformed or does not represent a
 * real calendar date.
 */
DateValue parseLiteralDate(const char * literal);

/** Returns the system's current date as a YYYYMMDD integer ("hoy"). */
DateValue today(void);

/** Returns the day before the system's current date ("ayer"), as YYYYMMDD. */
DateValue yesterday(void);

/** Returns the day after the system's current date ("mañana"), as YYYYMMDD. */
DateValue tomorrow(void);

/**
 * Writes the SQL/ISO literal "YYYY-MM-DD" of a valid YYYYMMDD date into the
 * caller-provided buffer, which must hold at least ISO_DATE_BUFFER_SIZE bytes.
 * Used by code generation to emit concrete dates into the SQL script.
 */
void formatDateValueIso(const DateValue date, char * buffer);

/**
 * Returns the given YYYYMMDD date shifted by "months" (negative shifts back).
 * The day-of-month is clamped to the last valid day of the resulting month
 * (e.g., 31-01 + 1 month -> 28/29-02), so the result is always a real date.
 */
DateValue addMonths(const DateValue date, const int months);

/**
 * Returns the given YYYYMMDD date shifted by "days" (negative shifts back),
 * normalizing across month and year boundaries.
 */
DateValue addDays(const DateValue date, const int days);

#endif
