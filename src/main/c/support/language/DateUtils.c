#include "DateUtils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* PRIVATE FUNCTIONS */

static DateValue _toDateValue(const struct tm * time) {
	return (time->tm_year + 1900) * 10000
		+ (time->tm_mon + 1) * 100
		+ time->tm_mday;
}

// fecha relativa a hoy, corrida "dayOffset" dias. mktime normaliza los bordes
// de mes/anio, asi que -1/+1 quedan bien (ej. el dia antes del 1ro).
static DateValue _relativeDate(const int dayOffset) {
	const time_t now = time(NULL);
	struct tm local;
	localtime_r(&now, &local);
	local.tm_mday += dayOffset;
	local.tm_isdst = -1;
	mktime(&local);
	return _toDateValue(&local);
}

/* PUBLIC FUNCTIONS */

bool isLeapYear(const int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(const int month, const int year) {
	static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month < 1 || month > 12) {
		return 0;
	}
	if (month == 2 && isLeapYear(year)) {
		return 29;
	}
	return days[month - 1];
}

bool isValidCalendarDate(const int day, const int month, const int year) {
	if (year < 1 || month < 1 || month > 12) {
		return false;
	}
	return day >= 1 && day <= daysInMonth(month, year);
}

DateValue parseLiteralDate(const char * literal) {
	if (literal == NULL || strlen(literal) != 10) {
		return INVALID_DATE_VALUE;
	}
	int day = 0;
	int month = 0;
	int year = 0;
	char trailing = '\0';
	// el "%c" final detecta caracteres de sobra mas alla de "DD-MM-YYYY"
	const int matched = sscanf(literal, "%2d-%2d-%4d%c", &day, &month, &year, &trailing);
	if (matched != 3) {
		return INVALID_DATE_VALUE;
	}
	if (!isValidCalendarDate(day, month, year)) {
		return INVALID_DATE_VALUE;
	}
	return year * 10000 + month * 100 + day;
}

DateValue today(void) {
	return _relativeDate(0);
}

DateValue yesterday(void) {
	return _relativeDate(-1);
}

DateValue tomorrow(void) {
	return _relativeDate(+1);
}

void formatDateValueIso(const DateValue date, char * buffer) {
	const int year = date / 10000;
	const int month = (date / 100) % 100;
	const int day = date % 100;
	sprintf(buffer, "%04d-%02d-%02d", year, month, day);
}

DateValue addMonths(const DateValue date, const int months) {
	const int year = date / 10000;
	const int month = (date / 100) % 100;
	int day = date % 100;
	// indice de mes base 0 para que los corrimientos negativos dividan bien
	int total = (year * 12 + (month - 1)) + months;
	int newYear = total / 12;
	int newMonth = total % 12;
	if (newMonth < 0) {
		newMonth += 12;
		newYear -= 1;
	}
	newMonth += 1;
	const int maxDay = daysInMonth(newMonth, newYear);
	if (day > maxDay) {
		day = maxDay;
	}
	return newYear * 10000 + newMonth * 100 + day;
}

DateValue addDays(const DateValue date, const int days) {
	struct tm time;
	memset(&time, 0, sizeof(time));
	time.tm_year = (date / 10000) - 1900;
	time.tm_mon = ((date / 100) % 100) - 1;
	time.tm_mday = (date % 100) + days;
	time.tm_isdst = -1;
	mktime(&time);
	return _toDateValue(&time);
}
