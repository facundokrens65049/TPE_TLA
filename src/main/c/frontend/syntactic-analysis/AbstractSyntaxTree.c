#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyReportFormat(ReportFormat * format) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (format != NULL) {
		free(format);
	}
}

void destroyDateValue(DateValue * dateValue) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (dateValue != NULL) {
		if (dateValue->kind == DATE_VALUE_ABSOLUTE) {
			free(dateValue->absoluteText);
		}
		free(dateValue);
	}
}

void destroyEditField(EditField * field) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (field != NULL) {
		switch (field->kind) {
			case EDIT_FIELD_AMOUNT:
				break;
			case EDIT_FIELD_CATEGORY:
				free(field->categoryId);
				break;
			case EDIT_FIELD_DATE_VALUE:
				destroyDateValue(field->dateValue);
				break;
			case EDIT_FIELD_DESCRIPTION:
				free(field->descriptionText);
				break;
		}
		free(field);
	}
}

void destroyEditFieldList(EditFieldList * list) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (list != NULL) {
		destroyEditField(list->field);
		destroyEditFieldList(list->next);
		free(list);
	}
}

void destroyPredefinedPeriod(PredefinedPeriod * period) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (period != NULL) {
		free(period);
	}
}

void destroyDateFilter(DateFilter * filter) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (filter != NULL) {
		switch (filter->kind) {
			case DATE_FILTER_RANGE:
				destroyDateValue(filter->rangeFrom);
				destroyDateValue(filter->rangeTo);
				break;
			case DATE_FILTER_PREDEFINED_PERIOD:
				destroyPredefinedPeriod(filter->period);
				break;
		}
		free(filter);
	}
}

void destroyOptionalDescription(OptionalDescription * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional->text);
		free(optional);
	}
}

void destroyOptionalEndDate(OptionalEndDate * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDateValue(optional->dateValue);
		free(optional);
	}
}

void destroyOptionalStartDate(OptionalStartDate * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDateValue(optional->dateValue);
		free(optional);
	}
}

void destroyOptionalOperationDate(OptionalOperationDate * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDateValue(optional->dateValue);
		free(optional);
	}
}

void destroyOptionalCategory(OptionalCategory * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional->identifier);
		free(optional);
	}
}

void destroyOptionalInstallments(OptionalInstallments * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional);
	}
}

void destroyFinalizeStatement(FinalizeStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		free(statement);
	}
}

void destroyReportStatement(ReportStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyReportFormat(statement->format);
		destroyDateFilter(statement->dateFilter);
		free(statement);
	}
}

void destroyDeleteStatement(DeleteStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		free(statement);
	}
}

void destroyEditStatement(EditStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyEditFieldList(statement->fields);
		free(statement);
	}
}

void destroyQueryStatement(QueryStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyDateFilter(statement->dateFilter);
		free(statement);
	}
}

void destroySubscriptionStatement(SubscriptionStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyPredefinedPeriod(statement->period);
		destroyOptionalCategory(statement->optionalCategory);
		destroyOptionalStartDate(statement->optionalStartDate);
		destroyOptionalEndDate(statement->optionalEndDate);
		destroyOptionalDescription(statement->optionalDescription);
		free(statement);
	}
}

void destroyIncomeStatement(IncomeStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyOptionalCategory(statement->optionalCategory);
		destroyOptionalOperationDate(statement->optionalOperationDate);
		destroyOptionalDescription(statement->optionalDescription);
		free(statement);
	}
}

void destroyExpenseStatement(ExpenseStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyOptionalInstallments(statement->optionalInstallments);
		destroyOptionalCategory(statement->optionalCategory);
		destroyOptionalOperationDate(statement->optionalOperationDate);
		destroyOptionalDescription(statement->optionalDescription);
		free(statement);
	}
}

void destroyCurrencyStatement(CurrencyStatement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		free(statement->identifier);
		free(statement);
	}
}

void destroyStatement(Statement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		switch (statement->kind) {
			case STATEMENT_CURRENCY:
				destroyCurrencyStatement(statement->currencyStatement);
				break;
			case STATEMENT_EXPENSE:
				destroyExpenseStatement(statement->expenseStatement);
				break;
			case STATEMENT_INCOME:
				destroyIncomeStatement(statement->incomeStatement);
				break;
			case STATEMENT_SUBSCRIPTION:
				destroySubscriptionStatement(statement->subscriptionStatement);
				break;
			case STATEMENT_QUERY:
				destroyQueryStatement(statement->queryStatement);
				break;
			case STATEMENT_EDIT:
				destroyEditStatement(statement->editStatement);
				break;
			case STATEMENT_DELETE:
				destroyDeleteStatement(statement->deleteStatement);
				break;
			case STATEMENT_REPORT:
				destroyReportStatement(statement->reportStatement);
				break;
			case STATEMENT_FINALIZE:
				destroyFinalizeStatement(statement->finalizeStatement);
				break;
		}
		free(statement);
	}
}

void destroyStatementList(StatementList * list) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (list != NULL) {
		destroyStatement(list->statement);
		destroyStatementList(list->next);
		free(list);
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyStatementList(program->statements);
		free(program);
	}
}
