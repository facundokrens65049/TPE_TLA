#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

typedef enum ReportFormatKind ReportFormatKind;
typedef enum DateValueKind DateValueKind;
typedef enum PredefinedPeriodKind PredefinedPeriodKind;
typedef enum DateFilterKind DateFilterKind;
typedef enum EditFieldKind EditFieldKind;
typedef enum StatementKind StatementKind;

typedef struct ReportFormat ReportFormat;
typedef struct DateValue DateValue;
typedef struct EditField EditField;
typedef struct EditFieldList EditFieldList;
typedef struct PredefinedPeriod PredefinedPeriod;
typedef struct DateFilter DateFilter;
typedef struct OptionalDescription OptionalDescription;
typedef struct OptionalEndDate OptionalEndDate;
typedef struct OptionalStartDate OptionalStartDate;
typedef struct OptionalOperationDate OptionalOperationDate;
typedef struct OptionalCategory OptionalCategory;
typedef struct OptionalInstallments OptionalInstallments;
typedef struct FinalizeStatement FinalizeStatement;
typedef struct ReportStatement ReportStatement;
typedef struct DeleteStatement DeleteStatement;
typedef struct EditStatement EditStatement;
typedef struct QueryStatement QueryStatement;
typedef struct SubscriptionStatement SubscriptionStatement;
typedef struct IncomeStatement IncomeStatement;
typedef struct ExpenseStatement ExpenseStatement;
typedef struct CurrencyStatement CurrencyStatement;
typedef struct Statement Statement;
typedef struct StatementList StatementList;
typedef struct Program Program;

enum ReportFormatKind {
	REPORT_FORMAT_HTML,
	REPORT_FORMAT_PLAIN_TEXT,
	REPORT_FORMAT_PDF
};

enum DateValueKind {
	DATE_VALUE_ABSOLUTE,
	DATE_VALUE_TODAY,
	DATE_VALUE_YESTERDAY,
	DATE_VALUE_TOMORROW
};

enum PredefinedPeriodKind {
	PREDEFINED_PERIOD_MONTHLY,
	PREDEFINED_PERIOD_WEEKLY,
	PREDEFINED_PERIOD_YEARLY
};

enum DateFilterKind {
	DATE_FILTER_RANGE,
	DATE_FILTER_PREDEFINED_PERIOD
};

enum EditFieldKind {
	EDIT_FIELD_AMOUNT,
	EDIT_FIELD_CATEGORY,
	EDIT_FIELD_DATE_VALUE,
	EDIT_FIELD_DESCRIPTION
};

enum StatementKind {
	STATEMENT_CURRENCY,
	STATEMENT_EXPENSE,
	STATEMENT_INCOME,
	STATEMENT_SUBSCRIPTION,
	STATEMENT_QUERY,
	STATEMENT_EDIT,
	STATEMENT_DELETE,
	STATEMENT_REPORT,
	STATEMENT_FINALIZE
};

struct ReportFormat {
	ReportFormatKind kind;
};

struct DateValue {
	DateValueKind kind;
	union {
		char * absoluteText;
	};
};

struct EditField {
	EditFieldKind kind;
	union {
		int amount;
		char * categoryId;
		DateValue * dateValue;
		char * descriptionText;
	};
};

struct EditFieldList {
	EditField * field;
	EditFieldList * next;
};

struct PredefinedPeriod {
	PredefinedPeriodKind kind;
};

struct DateFilter {
	DateFilterKind kind;
	union {
		struct {
			DateValue * rangeFrom;
			DateValue * rangeTo;
		};
		PredefinedPeriod * period;
	};
};

struct OptionalDescription {
	char * text;
};

struct OptionalEndDate {
	DateValue * dateValue;
};

struct OptionalStartDate {
	DateValue * dateValue;
};

struct OptionalOperationDate {
	DateValue * dateValue;
};

struct OptionalCategory {
	char * identifier;
};

struct OptionalInstallments {
	int count;
};

struct FinalizeStatement {
	int operationId;
};

struct ReportStatement {
	ReportFormat * format;
	DateFilter * dateFilter;
};

struct DeleteStatement {
	int operationId;
};

struct EditStatement {
	int operationId;
	EditFieldList * fields;
};

struct QueryStatement {
	DateFilter * dateFilter;
};

struct SubscriptionStatement {
	int amount;
	PredefinedPeriod * period;
	OptionalCategory * optionalCategory;
	OptionalStartDate * optionalStartDate;
	OptionalEndDate * optionalEndDate;
	OptionalDescription * optionalDescription;
};

struct IncomeStatement {
	int amount;
	OptionalCategory * optionalCategory;
	OptionalOperationDate * optionalOperationDate;
	OptionalDescription * optionalDescription;
};

struct ExpenseStatement {
	int amount;
	OptionalInstallments * optionalInstallments;
	OptionalCategory * optionalCategory;
	OptionalOperationDate * optionalOperationDate;
	OptionalDescription * optionalDescription;
};

struct CurrencyStatement {
	char * identifier;
};

struct Statement {
	StatementKind kind;
	union {
		CurrencyStatement * currencyStatement;
		ExpenseStatement * expenseStatement;
		IncomeStatement * incomeStatement;
		SubscriptionStatement * subscriptionStatement;
		QueryStatement * queryStatement;
		EditStatement * editStatement;
		DeleteStatement * deleteStatement;
		ReportStatement * reportStatement;
		FinalizeStatement * finalizeStatement;
	};
};

struct StatementList {
	Statement * statement;
	StatementList * next;
};

struct Program {
	StatementList * statements;
};

void destroyReportFormat(ReportFormat * format);
void destroyDateValue(DateValue * dateValue);
void destroyEditField(EditField * field);
void destroyEditFieldList(EditFieldList * list);
void destroyPredefinedPeriod(PredefinedPeriod * period);
void destroyDateFilter(DateFilter * filter);
void destroyOptionalDescription(OptionalDescription * optional);
void destroyOptionalEndDate(OptionalEndDate * optional);
void destroyOptionalStartDate(OptionalStartDate * optional);
void destroyOptionalOperationDate(OptionalOperationDate * optional);
void destroyOptionalCategory(OptionalCategory * optional);
void destroyOptionalInstallments(OptionalInstallments * optional);
void destroyFinalizeStatement(FinalizeStatement * statement);
void destroyReportStatement(ReportStatement * statement);
void destroyDeleteStatement(DeleteStatement * statement);
void destroyEditStatement(EditStatement * statement);
void destroyQueryStatement(QueryStatement * statement);
void destroySubscriptionStatement(SubscriptionStatement * statement);
void destroyIncomeStatement(IncomeStatement * statement);
void destroyExpenseStatement(ExpenseStatement * statement);
void destroyCurrencyStatement(CurrencyStatement * statement);
void destroyStatement(Statement * statement);
void destroyStatementList(StatementList * list);
void destroyProgram(Program * program);

#endif
