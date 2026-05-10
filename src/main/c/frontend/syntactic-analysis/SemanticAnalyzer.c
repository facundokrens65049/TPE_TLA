#include "SemanticAnalyzer.h"

#include "../../support/logging/Logger.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static bool _isLeapYear(const int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int _daysInMonth(const int month, const int year) {
	static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month == 2 && _isLeapYear(year)) {
		return 29;
	}
	return days[month - 1];
}

static bool _validYmd(const int day, const int month, const int year) {
	if (year < 1 || month < 1 || month > 12) {
		return false;
	}
	if (day < 1 || day > _daysInMonth(month, year)) {
		return false;
	}
	return true;
}

static bool _parseDdMmYyyy(const char * s, int * outDay, int * outMonth, int * outYear) {
	if (s == NULL || strlen(s) != 10u || s[2] != '-' || s[5] != '-') {
		return false;
	}
	for (int i = 0; i < 10; ++i) {
		if (i == 2 || i == 5) {
			continue;
		}
		if (!isdigit((unsigned char) s[i])) {
			return false;
		}
	}
	int d = 0;
	int m = 0;
	int y = 0;
	if (sscanf(s, "%2d-%2d-%4d", &d, &m, &y) != 3) {
		return false;
	}
	*outDay = d;
	*outMonth = m;
	*outYear = y;
	return true;
}

static bool _absoluteDateValueValid(DateValue * dateValue) {
	if (dateValue == NULL || dateValue->kind != DATE_VALUE_ABSOLUTE) {
		return true;
	}
	int d = 0;
	int m = 0;
	int y = 0;
	if (!_parseDdMmYyyy(dateValue->absoluteText, &d, &m, &y)) {
		return false;
	}
	return _validYmd(d, m, y);
}

static void _timeToYmd(const time_t t, int * y, int * m, int * d) {
	struct tm * const tm = localtime(&t);
	*y = tm->tm_year + 1900;
	*m = tm->tm_mon + 1;
	*d = tm->tm_mday;
}

static int _ymdToComparable(const int y, const int m, const int d) {
	return y * 10000 + m * 100 + d;
}

static bool _dateValueToComparable(DateValue * dateValue, const time_t ref, int * out) {
	if (dateValue == NULL || out == NULL) {
		return false;
	}
	switch (dateValue->kind) {
		case DATE_VALUE_ABSOLUTE: {
			int d = 0;
			int m = 0;
			int y = 0;
			if (!_parseDdMmYyyy(dateValue->absoluteText, &d, &m, &y) || !_validYmd(d, m, y)) {
				return false;
			}
			*out = _ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_TODAY: {
			int y = 0;
			int m = 0;
			int d = 0;
			_timeToYmd(ref, &y, &m, &d);
			*out = _ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_YESTERDAY: {
			const time_t t = ref - (time_t) 86400;
			int y = 0;
			int m = 0;
			int d = 0;
			_timeToYmd(t, &y, &m, &d);
			*out = _ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_TOMORROW: {
			const time_t t = ref + (time_t) 86400;
			int y = 0;
			int m = 0;
			int d = 0;
			_timeToYmd(t, &y, &m, &d);
			*out = _ymdToComparable(y, m, d);
			return true;
		}
		default:
			return false;
	}
}

static bool _validateDateFilter(DateFilter * filter, const time_t ref, Logger * logger) {
	if (filter == NULL) {
		return true;
	}
	if (filter->kind == DATE_FILTER_PREDEFINED_PERIOD) {
		return true;
	}
	if (!_absoluteDateValueValid(filter->rangeFrom) || !_absoluteDateValueValid(filter->rangeTo)) {
		logError(logger, "Semantic error: invalid absolute date in range.");
		return false;
	}
	int fromCmp = 0;
	int toCmp = 0;
	if (!_dateValueToComparable(filter->rangeFrom, ref, &fromCmp) || !_dateValueToComparable(filter->rangeTo, ref, &toCmp)) {
		logError(logger, "Semantic error: could not compare date range.");
		return false;
	}
	if (fromCmp > toCmp) {
		logError(logger, "Semantic error: start date is after end date.");
		return false;
	}
	return true;
}

static bool _validateEditFieldList(EditFieldList * fields, Logger * logger) {
	for (EditFieldList * node = fields; node != NULL; node = node->next) {
		if (node->field == NULL) {
			continue;
		}
		EditField * const f = node->field;
		switch (f->kind) {
			case EDIT_FIELD_AMOUNT:
				if (f->amount <= 0) {
					logError(logger, "Semantic error: edited amount must be positive.");
					return false;
				}
				break;
			case EDIT_FIELD_DATE_VALUE:
				if (!_absoluteDateValueValid(f->dateValue)) {
					logError(logger, "Semantic error: invalid date in edit field.");
					return false;
				}
				break;
			case EDIT_FIELD_CATEGORY:
			case EDIT_FIELD_DESCRIPTION:
				break;
		}
	}
	return true;
}

static bool _validateStatement(Statement * statement, const time_t ref, Logger * logger) {
	if (statement == NULL) {
		return true;
	}
	switch (statement->kind) {
		case STATEMENT_CURRENCY:
			return true;
		case STATEMENT_EXPENSE: {
			ExpenseStatement * const e = statement->expenseStatement;
			if (e->amount <= 0) {
				logError(logger, "Semantic error: expense amount must be positive.");
				return false;
			}
			if (e->optionalInstallments != NULL && e->optionalInstallments->count < 1) {
				logError(logger, "Semantic error: installment count must be at least 1.");
				return false;
			}
			if (e->optionalOperationDate != NULL && !_absoluteDateValueValid(e->optionalOperationDate->dateValue)) {
				logError(logger, "Semantic error: invalid date on expense.");
				return false;
			}
			return true;
		}
		case STATEMENT_INCOME: {
			IncomeStatement * const i = statement->incomeStatement;
			if (i->amount <= 0) {
				logError(logger, "Semantic error: income amount must be positive.");
				return false;
			}
			if (i->optionalOperationDate != NULL && !_absoluteDateValueValid(i->optionalOperationDate->dateValue)) {
				logError(logger, "Semantic error: invalid date on income.");
				return false;
			}
			return true;
		}
		case STATEMENT_SUBSCRIPTION: {
			SubscriptionStatement * const s = statement->subscriptionStatement;
			if (s->amount <= 0) {
				logError(logger, "Semantic error: subscription amount must be positive.");
				return false;
			}
			if (s->optionalStartDate != NULL && !_absoluteDateValueValid(s->optionalStartDate->dateValue)) {
				logError(logger, "Semantic error: invalid subscription start date.");
				return false;
			}
			if (s->optionalEndDate != NULL && !_absoluteDateValueValid(s->optionalEndDate->dateValue)) {
				logError(logger, "Semantic error: invalid subscription end date.");
				return false;
			}
			if (s->optionalStartDate != NULL && s->optionalEndDate != NULL) {
				int a = 0;
				int b = 0;
				if (!_dateValueToComparable(s->optionalStartDate->dateValue, ref, &a) || !_dateValueToComparable(s->optionalEndDate->dateValue, ref, &b)) {
					logError(logger, "Semantic error: could not compare subscription dates.");
					return false;
				}
				if (a > b) {
					logError(logger, "Semantic error: subscription end date is before start date.");
					return false;
				}
			}
			return true;
		}
		case STATEMENT_QUERY:
			return _validateDateFilter(statement->queryStatement->dateFilter, ref, logger);
		case STATEMENT_EDIT: {
			EditStatement * const e = statement->editStatement;
			if (e->operationId <= 0) {
				logError(logger, "Semantic error: operation id must be positive.");
				return false;
			}
			return _validateEditFieldList(e->fields, logger);
		}
		case STATEMENT_DELETE:
			if (statement->deleteStatement->operationId <= 0) {
				logError(logger, "Semantic error: operation id must be positive.");
				return false;
			}
			return true;
		case STATEMENT_REPORT:
			return _validateDateFilter(statement->reportStatement->dateFilter, ref, logger);
		case STATEMENT_FINALIZE:
			if (statement->finalizeStatement->operationId <= 0) {
				logError(logger, "Semantic error: operation id must be positive.");
				return false;
			}
			return true;
		default:
			return true;
	}
}

CompilationStatus validateProgram(Program * program) {
	Logger * const logger = createLogger("SemanticAnalyzer");
	if (logger == NULL) {
		return FAILED;
	}
	if (program == NULL || program->statements == NULL) {
		destroyLogger(logger);
		return SUCCEEDED;
	}
	const time_t ref = time(NULL);
	for (StatementList * node = program->statements; node != NULL; node = node->next) {
		if (!_validateStatement(node->statement, ref, logger)) {
			destroyLogger(logger);
			return FAILED;
		}
	}
	destroyLogger(logger);
	return SUCCEEDED;
}
