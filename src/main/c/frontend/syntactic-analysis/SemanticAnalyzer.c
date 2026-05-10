#include "SemanticAnalyzer.h"

#include "../../support/logging/Logger.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static bool isLeapYear(const int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int daysInMonth(const int month, const int year) {
	static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month == 2 && isLeapYear(year)) {
		return 29;
	}
	return days[month - 1];
}

static bool validYmd(const int day, const int month, const int year) {
	if (year < 1 || month < 1 || month > 12) {
		return false;
	}
	if (day < 1 || day > daysInMonth(month, year)) {
		return false;
	}
	return true;
}

static bool parseDdMmYyyy(const char * s, int * outDay, int * outMonth, int * outYear) {
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

static bool absoluteDateValueValid(DateValue * dateValue) {
	if (dateValue == NULL || dateValue->kind != DATE_VALUE_ABSOLUTE) {
		return true;
	}
	int d = 0;
	int m = 0;
	int y = 0;
	if (!parseDdMmYyyy(dateValue->absoluteText, &d, &m, &y)) {
		return false;
	}
	return validYmd(d, m, y);
}

static void timeToYmd(const time_t t, int * y, int * m, int * d) {
	struct tm * const tm = localtime(&t);
	*y = tm->tm_year + 1900;
	*m = tm->tm_mon + 1;
	*d = tm->tm_mday;
}

static int ymdToComparable(const int y, const int m, const int d) {
	return y * 10000 + m * 100 + d;
}

static bool dateValueToComparable(DateValue * dateValue, const time_t ref, int * out) {
	if (dateValue == NULL || out == NULL) {
		return false;
	}
	switch (dateValue->kind) {
		case DATE_VALUE_ABSOLUTE: {
			int d = 0;
			int m = 0;
			int y = 0;
			if (!parseDdMmYyyy(dateValue->absoluteText, &d, &m, &y) || !validYmd(d, m, y)) {
				return false;
			}
			*out = ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_TODAY: {
			int y = 0;
			int m = 0;
			int d = 0;
			timeToYmd(ref, &y, &m, &d);
			*out = ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_YESTERDAY: {
			const time_t t = ref - (time_t) 86400;
			int y = 0;
			int m = 0;
			int d = 0;
			timeToYmd(t, &y, &m, &d);
			*out = ymdToComparable(y, m, d);
			return true;
		}
		case DATE_VALUE_TOMORROW: {
			const time_t t = ref + (time_t) 86400;
			int y = 0;
			int m = 0;
			int d = 0;
			timeToYmd(t, &y, &m, &d);
			*out = ymdToComparable(y, m, d);
			return true;
		}
		default:
			return false;
	}
}

static bool validateDateFilter(DateFilter * filter, const time_t ref, Logger * logger) {
	if (filter == NULL) {
		return true;
	}
	if (filter->kind == DATE_FILTER_PREDEFINED_PERIOD) {
		return true;
	}
	if (!absoluteDateValueValid(filter->rangeFrom) || !absoluteDateValueValid(filter->rangeTo)) {
		logError(logger, "Semantic error: invalid absolute date in range.");
		return false;
	}
	int fromCmp = 0;
	int toCmp = 0;
	if (!dateValueToComparable(filter->rangeFrom, ref, &fromCmp) || !dateValueToComparable(filter->rangeTo, ref, &toCmp)) {
		logError(logger, "Semantic error: could not compare date range.");
		return false;
	}
	if (fromCmp > toCmp) {
		logError(logger, "Semantic error: start date is after end date.");
		return false;
	}
	return true;
}

static bool validateEditFieldList(EditFieldList * fields, Logger * logger) {
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
				if (!absoluteDateValueValid(f->dateValue)) {
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

static bool validateStatement(Statement * statement, const time_t ref, Logger * logger) {
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
			if (e->optionalOperationDate != NULL && !absoluteDateValueValid(e->optionalOperationDate->dateValue)) {
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
			if (i->optionalOperationDate != NULL && !absoluteDateValueValid(i->optionalOperationDate->dateValue)) {
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
			if (s->optionalStartDate != NULL && !absoluteDateValueValid(s->optionalStartDate->dateValue)) {
				logError(logger, "Semantic error: invalid subscription start date.");
				return false;
			}
			if (s->optionalEndDate != NULL && !absoluteDateValueValid(s->optionalEndDate->dateValue)) {
				logError(logger, "Semantic error: invalid subscription end date.");
				return false;
			}
			if (s->optionalStartDate != NULL && s->optionalEndDate != NULL) {
				int a = 0;
				int b = 0;
				if (!dateValueToComparable(s->optionalStartDate->dateValue, ref, &a) || !dateValueToComparable(s->optionalEndDate->dateValue, ref, &b)) {
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
			return validateDateFilter(statement->queryStatement->dateFilter, ref, logger);
		case STATEMENT_EDIT: {
			EditStatement * const e = statement->editStatement;
			if (e->operationId <= 0) {
				logError(logger, "Semantic error: operation id must be positive.");
				return false;
			}
			return validateEditFieldList(e->fields, logger);
		}
		case STATEMENT_DELETE:
			if (statement->deleteStatement->operationId <= 0) {
				logError(logger, "Semantic error: operation id must be positive.");
				return false;
			}
			return true;
		case STATEMENT_REPORT:
			return validateDateFilter(statement->reportStatement->dateFilter, ref, logger);
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
		if (!validateStatement(node->statement, ref, logger)) {
			destroyLogger(logger);
			return FAILED;
		}
	}
	destroyLogger(logger);
	return SUCCEEDED;
}
