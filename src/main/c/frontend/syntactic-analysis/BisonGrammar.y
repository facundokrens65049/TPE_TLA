%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

void yyerror(const YYLTYPE * location, const char * message) {}

%}

%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	signed int integer;
	TokenLabel token;
	char * string;

	Program * program;
	StatementList * statementList;
	Statement * statement;
	CurrencyStatement * currencyStmt;
	ExpenseStatement * expenseStmt;
	IncomeStatement * incomeStmt;
	SubscriptionStatement * subscriptionStmt;
	QueryStatement * queryStmt;
	EditStatement * editStmt;
	DeleteStatement * deleteStmt;
	ReportStatement * reportStmt;
	FinalizeStatement * finalizeStmt;
	OptionalInstallments * optionalInstallments;
	OptionalCategory * optionalCategory;
	OptionalOperationDate * optionalOperationDate;
	OptionalStartDate * optionalStartDate;
	OptionalEndDate * optionalEndDate;
	OptionalDescription * optionalDescription;
	DateFilter * dateFilter;
	PredefinedPeriod * predefinedPeriod;
	EditFieldList * editFields;
	EditField * editField;
	DateValue * dateValue;
	ReportFormat * reportFormat;
}

%destructor { free($$); } <string>
%destructor { destroyStatementList($$); } <statementList>
%destructor { destroyStatement($$); } <statement>
%destructor { destroyCurrencyStatement($$); } <currencyStmt>
%destructor { destroyExpenseStatement($$); } <expenseStmt>
%destructor { destroyIncomeStatement($$); } <incomeStmt>
%destructor { destroySubscriptionStatement($$); } <subscriptionStmt>
%destructor { destroyQueryStatement($$); } <queryStmt>
%destructor { destroyEditStatement($$); } <editStmt>
%destructor { destroyDeleteStatement($$); } <deleteStmt>
%destructor { destroyReportStatement($$); } <reportStmt>
%destructor { destroyFinalizeStatement($$); } <finalizeStmt>
%destructor { destroyOptionalInstallments($$); } <optionalInstallments>
%destructor { destroyOptionalCategory($$); } <optionalCategory>
%destructor { destroyOptionalOperationDate($$); } <optionalOperationDate>
%destructor { destroyOptionalStartDate($$); } <optionalStartDate>
%destructor { destroyOptionalEndDate($$); } <optionalEndDate>
%destructor { destroyOptionalDescription($$); } <optionalDescription>
%destructor { destroyDateFilter($$); } <dateFilter>
%destructor { destroyPredefinedPeriod($$); } <predefinedPeriod>
%destructor { destroyEditFieldList($$); } <editFields>
%destructor { destroyEditField($$); } <editField>
%destructor { destroyDateValue($$); } <dateValue>
%destructor { destroyReportFormat($$); } <reportFormat>

%token <token> IGNORED
%token <token> UNKNOWN
%token <token> OPEN_COMMENT
%token <token> CLOSE_COMMENT

%token <integer> NUMBER
%token <string> STRING
%token <string> DATE
%token <string> ID
%token <token> CURRENCY
%token <token> EXPENSE
%token <token> INCOME
%token <token> SUBSCRIPTION
%token <token> QUERY
%token <token> EDIT
%token <token> DELETE
%token <token> REPORT
%token <token> FINALIZE
%token <token> INSTALLMENTS
%token <token> CATEGORY
%token <token> DATE_KEYWORD
%token <token> DESCRIPTION_KEYWORD
%token <token> AMOUNT_KEYWORD
%token <token> PERIOD_MONTHLY
%token <token> PERIOD_WEEKLY
%token <token> PERIOD_YEARLY
%token <token> KW_TODAY
%token <token> KW_YESTERDAY
%token <token> KW_TOMORROW
%token <token> KW_FROM
%token <token> KW_UNTIL
%token <token> HTML
%token <token> PLAIN_TEXT
%token <token> PDF

%type <program> program
%type <statementList> statementList
%type <statement> statement
%type <currencyStmt> currencyStmt
%type <expenseStmt> expenseStmt
%type <incomeStmt> incomeStmt
%type <subscriptionStmt> subscriptionStmt
%type <queryStmt> queryStmt
%type <editStmt> editStmt
%type <deleteStmt> deleteStmt
%type <reportStmt> reportStmt
%type <finalizeStmt> finalizeStmt
%type <optionalInstallments> optionalInstallments
%type <optionalCategory> optionalCategory
%type <optionalOperationDate> optionalOperationDate
%type <optionalStartDate> optionalStartDate
%type <optionalEndDate> optionalEndDate
%type <optionalDescription> optionalDescription
%type <dateFilter> dateFilter
%type <predefinedPeriod> predefinedPeriod
%type <editFields> editFields
%type <editField> editField
%type <dateValue> dateValue
%type <reportFormat> reportFormat

%%

program: statementList						{ $$ = buildProgramSemanticAction($1); }
	;

statementList: statementList statement			{ $$ = appendStatementListSemanticAction($1, $2); }
	| statement					{ $$ = singletonStatementListSemanticAction($1); }
	;

statement: currencyStmt 				{ $$ = wrapCurrencyStatementSemanticAction($1); }
	| expenseStmt 					{ $$ = wrapExpenseStatementSemanticAction($1); }
	| incomeStmt 					{ $$ = wrapIncomeStatementSemanticAction($1); }
	| subscriptionStmt 				{ $$ = wrapSubscriptionStatementSemanticAction($1); }
	| queryStmt 					{ $$ = wrapQueryStatementSemanticAction($1); }
	| editStmt 					{ $$ = wrapEditStatementSemanticAction($1); }
	| deleteStmt 					{ $$ = wrapDeleteStatementSemanticAction($1); }
	| reportStmt 					{ $$ = wrapReportStatementSemanticAction($1); }
	| finalizeStmt 					{ $$ = wrapFinalizeStatementSemanticAction($1); }
	;

currencyStmt: CURRENCY ID 				{ $$ = currencyStatementSemanticAction($2); }
	;

expenseStmt: EXPENSE NUMBER optionalInstallments optionalCategory optionalOperationDate optionalDescription
		{ $$ = expenseStatementSemanticAction($2, $3, $4, $5, $6); }
	;

incomeStmt: INCOME NUMBER optionalCategory optionalOperationDate optionalDescription
		{ $$ = incomeStatementSemanticAction($2, $3, $4, $5); }
	;

subscriptionStmt: SUBSCRIPTION NUMBER predefinedPeriod optionalCategory optionalStartDate optionalEndDate optionalDescription
		{ $$ = subscriptionStatementSemanticAction($2, $3, $4, $5, $6, $7); }
	;

queryStmt: QUERY dateFilter { $$ = queryStatementSemanticAction($2); }
	;

editStmt: EDIT NUMBER editFields { $$ = editStatementSemanticAction($2, $3); }
	;

deleteStmt: DELETE NUMBER { $$ = deleteStatementSemanticAction($2); }
	;

reportStmt: REPORT reportFormat dateFilter { $$ = reportStatementSemanticAction($2, $3); }
	;

finalizeStmt: FINALIZE NUMBER { $$ = finalizeStatementSemanticAction($2); }
	;

optionalInstallments: INSTALLMENTS NUMBER { $$ = presentOptionalInstallmentsSemanticAction($2); }
	| %empty { $$ = emptyOptionalInstallmentsSemanticAction(); }
	;

optionalCategory: CATEGORY ID { $$ = presentOptionalCategorySemanticAction($2); }
	| %empty { $$ = emptyOptionalCategorySemanticAction(); }
	;

optionalOperationDate: DATE_KEYWORD dateValue { $$ = presentOptionalOperationDateSemanticAction($2); }
	| %empty { $$ = emptyOptionalOperationDateSemanticAction(); }
	;

optionalStartDate: KW_FROM dateValue { $$ = presentOptionalStartDateSemanticAction($2); }
	| %empty { $$ = emptyOptionalStartDateSemanticAction(); }
	;

optionalEndDate: KW_UNTIL dateValue { $$ = presentOptionalEndDateSemanticAction($2); }
	| %empty { $$ = emptyOptionalEndDateSemanticAction(); }
	;

optionalDescription: DESCRIPTION_KEYWORD STRING { $$ = presentOptionalDescriptionSemanticAction($2); }
	| %empty { $$ = emptyOptionalDescriptionSemanticAction(); }
	;

dateFilter: KW_FROM dateValue KW_UNTIL dateValue { $$ = dateFilterRangeSemanticAction($2, $4); }
	| predefinedPeriod { $$ = dateFilterPredefinedSemanticAction($1); }
	;

predefinedPeriod: PERIOD_MONTHLY { $$ = predefinedPeriodSemanticAction(PREDEFINED_PERIOD_MONTHLY); }
	| PERIOD_WEEKLY { $$ = predefinedPeriodSemanticAction(PREDEFINED_PERIOD_WEEKLY); }
	| PERIOD_YEARLY { $$ = predefinedPeriodSemanticAction(PREDEFINED_PERIOD_YEARLY); }
	;

editFields: editFields editField { $$ = appendEditFieldListSemanticAction($1, $2); }
	| editField { $$ = singletonEditFieldListSemanticAction($1); }
	;

editField: AMOUNT_KEYWORD NUMBER { $$ = editFieldAmountSemanticAction($2); }
	| CATEGORY ID { $$ = editFieldCategorySemanticAction($2); }
	| DATE_KEYWORD dateValue { $$ = editFieldDateValueSemanticAction($2); }
	| DESCRIPTION_KEYWORD STRING { $$ = editFieldDescriptionSemanticAction($2); }
	;

dateValue: DATE { $$ = absoluteDateStringSemanticAction($1); }
	| KW_TODAY { $$ = relativeDateSemanticAction(DATE_VALUE_TODAY); }
	| KW_YESTERDAY { $$ = relativeDateSemanticAction(DATE_VALUE_YESTERDAY); }
	| KW_TOMORROW { $$ = relativeDateSemanticAction(DATE_VALUE_TOMORROW); }
	;

reportFormat: HTML { $$ = reportFormatSemanticAction(REPORT_FORMAT_HTML); }
	| PLAIN_TEXT { $$ = reportFormatSemanticAction(REPORT_FORMAT_PLAIN_TEXT); }
	| PDF { $$ = reportFormatSemanticAction(REPORT_FORMAT_PDF); }
	;

%%
