#include "BisonActions.h"

#include "../../support/language/String.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

static void _logParserAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

ReportFormat * reportFormatSemanticAction(ReportFormatKind kind) {
	_logParserAction(__FUNCTION__);
	ReportFormat * format = calloc(1, sizeof(ReportFormat));
	format->kind = kind;
	return format;
}

DateValue * relativeDateSemanticAction(DateValueKind kind) {
	_logParserAction(__FUNCTION__);
	DateValue * dateValue = calloc(1, sizeof(DateValue));
	dateValue->kind = kind;
	return dateValue;
}

DateValue * absoluteDateStringSemanticAction(const char * text) {
	_logParserAction(__FUNCTION__);
	DateValue * dateValue = calloc(1, sizeof(DateValue));
	dateValue->kind = DATE_VALUE_ABSOLUTE;
	dateValue->absoluteText = (char *) text;
	return dateValue;
}

EditField * editFieldDescriptionSemanticAction(const char * text) {
	_logParserAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_DESCRIPTION;
	field->descriptionText = (char *) text;
	return field;
}

EditField * editFieldDateValueSemanticAction(DateValue * dateValue) {
	_logParserAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_DATE_VALUE;
	field->dateValue = dateValue;
	return field;
}

EditField * editFieldCategorySemanticAction(const char * identifier) {
	_logParserAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_CATEGORY;
	char * normalized = normalizeFinancialCategory(identifier);
	free((void *) identifier);
	field->categoryId = normalized;
	return field;
}

EditField * editFieldAmountSemanticAction(const int amount) {
	_logParserAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_AMOUNT;
	field->amount = amount;
	return field;
}

EditFieldList * singletonEditFieldListSemanticAction(EditField * field) {
	_logParserAction(__FUNCTION__);
	EditFieldList * list = calloc(1, sizeof(EditFieldList));
	list->field = field;
	return list;
}

EditFieldList * appendEditFieldListSemanticAction(EditFieldList * list, EditField * field) {
	_logParserAction(__FUNCTION__);
	EditFieldList * newHead = calloc(1, sizeof(EditFieldList));
	newHead->field = field;
	newHead->next = list;
	return newHead;
}

PredefinedPeriod * predefinedPeriodSemanticAction(PredefinedPeriodKind kind) {
	_logParserAction(__FUNCTION__);
	PredefinedPeriod * period = calloc(1, sizeof(PredefinedPeriod));
	period->kind = kind;
	return period;
}

DateFilter * dateFilterRangeSemanticAction(DateValue * fromDate, DateValue * toDate) {
	_logParserAction(__FUNCTION__);
	DateFilter * filter = calloc(1, sizeof(DateFilter));
	filter->kind = DATE_FILTER_RANGE;
	filter->rangeFrom = fromDate;
	filter->rangeTo = toDate;
	return filter;
}

DateFilter * dateFilterPredefinedSemanticAction(PredefinedPeriod * period) {
	_logParserAction(__FUNCTION__);
	DateFilter * filter = calloc(1, sizeof(DateFilter));
	filter->kind = DATE_FILTER_PREDEFINED_PERIOD;
	filter->period = period;
	return filter;
}

OptionalDescription * emptyOptionalDescriptionSemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalDescription * presentOptionalDescriptionSemanticAction(const char * text) {
	_logParserAction(__FUNCTION__);
	OptionalDescription * optional = calloc(1, sizeof(OptionalDescription));
	optional->text = (char *) text;
	return optional;
}

OptionalEndDate * emptyOptionalEndDateSemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalEndDate * presentOptionalEndDateSemanticAction(DateValue * dateValue) {
	_logParserAction(__FUNCTION__);
	OptionalEndDate * optional = calloc(1, sizeof(OptionalEndDate));
	optional->dateValue = dateValue;
	return optional;
}

OptionalStartDate * emptyOptionalStartDateSemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalStartDate * presentOptionalStartDateSemanticAction(DateValue * dateValue) {
	_logParserAction(__FUNCTION__);
	OptionalStartDate * optional = calloc(1, sizeof(OptionalStartDate));
	optional->dateValue = dateValue;
	return optional;
}

OptionalOperationDate * emptyOptionalOperationDateSemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalOperationDate * presentOptionalOperationDateSemanticAction(DateValue * dateValue) {
	_logParserAction(__FUNCTION__);
	OptionalOperationDate * optional = calloc(1, sizeof(OptionalOperationDate));
	optional->dateValue = dateValue;
	return optional;
}

OptionalCategory * emptyOptionalCategorySemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalCategory * presentOptionalCategorySemanticAction(const char * identifier) {
	_logParserAction(__FUNCTION__);
	OptionalCategory * optional = calloc(1, sizeof(OptionalCategory));
	char * normalized = normalizeFinancialCategory(identifier);
	free((void *) identifier);
	optional->identifier = normalized;
	return optional;
}

OptionalInstallments * emptyOptionalInstallmentsSemanticAction() {
	_logParserAction(__FUNCTION__);
	return NULL;
}

OptionalInstallments * presentOptionalInstallmentsSemanticAction(const int count) {
	_logParserAction(__FUNCTION__);
	OptionalInstallments * optional = calloc(1, sizeof(OptionalInstallments));
	optional->count = count;
	return optional;
}

FinalizeStatement * finalizeStatementSemanticAction(const int operationId) {
	_logParserAction(__FUNCTION__);
	FinalizeStatement * stmt = calloc(1, sizeof(FinalizeStatement));
	stmt->operationId = operationId;
	return stmt;
}

ReportStatement * reportStatementSemanticAction(ReportFormat * format, DateFilter * dateFilter) {
	_logParserAction(__FUNCTION__);
	ReportStatement * stmt = calloc(1, sizeof(ReportStatement));
	stmt->format = format;
	stmt->dateFilter = dateFilter;
	return stmt;
}

DeleteStatement * deleteStatementSemanticAction(const int operationId) {
	_logParserAction(__FUNCTION__);
	DeleteStatement * stmt = calloc(1, sizeof(DeleteStatement));
	stmt->operationId = operationId;
	return stmt;
}

EditStatement * editStatementSemanticAction(const int operationId, EditFieldList * fields) {
	_logParserAction(__FUNCTION__);
	EditStatement * stmt = calloc(1, sizeof(EditStatement));
	stmt->operationId = operationId;
	stmt->fields = fields;
	return stmt;
}

QueryStatement * queryStatementSemanticAction(DateFilter * dateFilter) {
	_logParserAction(__FUNCTION__);
	QueryStatement * stmt = calloc(1, sizeof(QueryStatement));
	stmt->dateFilter = dateFilter;
	return stmt;
}

SubscriptionStatement * subscriptionStatementSemanticAction(const int amount, PredefinedPeriod * period, OptionalCategory * optionalCategory, OptionalStartDate * optionalStartDate, OptionalEndDate * optionalEndDate, OptionalDescription * optionalDescription) {
	_logParserAction(__FUNCTION__);
	SubscriptionStatement * stmt = calloc(1, sizeof(SubscriptionStatement));
	stmt->amount = amount;
	stmt->period = period;
	stmt->optionalCategory = optionalCategory;
	stmt->optionalStartDate = optionalStartDate;
	stmt->optionalEndDate = optionalEndDate;
	stmt->optionalDescription = optionalDescription;
	return stmt;
}

IncomeStatement * incomeStatementSemanticAction(const int amount, OptionalCategory * optionalCategory, OptionalOperationDate * optionalOperationDate, OptionalDescription * optionalDescription) {
	_logParserAction(__FUNCTION__);
	IncomeStatement * stmt = calloc(1, sizeof(IncomeStatement));
	stmt->amount = amount;
	stmt->optionalCategory = optionalCategory;
	stmt->optionalOperationDate = optionalOperationDate;
	stmt->optionalDescription = optionalDescription;
	return stmt;
}

ExpenseStatement * expenseStatementSemanticAction(const int amount, OptionalInstallments * optionalInstallments, OptionalCategory * optionalCategory, OptionalOperationDate * optionalOperationDate, OptionalDescription * optionalDescription) {
	_logParserAction(__FUNCTION__);
	ExpenseStatement * stmt = calloc(1, sizeof(ExpenseStatement));
	stmt->amount = amount;
	stmt->optionalInstallments = optionalInstallments;
	stmt->optionalCategory = optionalCategory;
	stmt->optionalOperationDate = optionalOperationDate;
	stmt->optionalDescription = optionalDescription;
	return stmt;
}

CurrencyStatement * currencyStatementSemanticAction(const char * identifier) {
	_logParserAction(__FUNCTION__);
	CurrencyStatement * stmt = calloc(1, sizeof(CurrencyStatement));
	stmt->identifier = (char *) identifier;
	return stmt;
}

Statement * wrapFinalizeStatementSemanticAction(FinalizeStatement * finalizeStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_FINALIZE;
	statement->finalizeStatement = finalizeStmt;
	return statement;
}

Statement * wrapReportStatementSemanticAction(ReportStatement * reportStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_REPORT;
	statement->reportStatement = reportStmt;
	return statement;
}

Statement * wrapDeleteStatementSemanticAction(DeleteStatement * deleteStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_DELETE;
	statement->deleteStatement = deleteStmt;
	return statement;
}

Statement * wrapEditStatementSemanticAction(EditStatement * editStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_EDIT;
	statement->editStatement = editStmt;
	return statement;
}

Statement * wrapQueryStatementSemanticAction(QueryStatement * queryStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_QUERY;
	statement->queryStatement = queryStmt;
	return statement;
}

Statement * wrapSubscriptionStatementSemanticAction(SubscriptionStatement * subscriptionStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_SUBSCRIPTION;
	statement->subscriptionStatement = subscriptionStmt;
	return statement;
}

Statement * wrapIncomeStatementSemanticAction(IncomeStatement * incomeStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_INCOME;
	statement->incomeStatement = incomeStmt;
	return statement;
}

Statement * wrapExpenseStatementSemanticAction(ExpenseStatement * expenseStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_EXPENSE;
	statement->expenseStatement = expenseStmt;
	return statement;
}

Statement * wrapCurrencyStatementSemanticAction(CurrencyStatement * currencyStmt) {
	_logParserAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_CURRENCY;
	statement->currencyStatement = currencyStmt;
	return statement;
}

StatementList * singletonStatementListSemanticAction(Statement * statement) {
	_logParserAction(__FUNCTION__);
	StatementList * list = calloc(1, sizeof(StatementList));
	list->statement = statement;
	list->next = NULL;
	return list;
}

StatementList * appendStatementListSemanticAction(StatementList * list, Statement * statement) {
	_logParserAction(__FUNCTION__);
	StatementList * newHead = calloc(1, sizeof(StatementList));
	newHead->statement = statement;
	newHead->next = list;
	return newHead;
}

Program * buildProgramSemanticAction(StatementList * statements) {
	_logParserAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->statements = statements;
	_compilerState->abstractSyntaxTree = program;
	return program;
}
