#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

ReportFormat * reportFormatSemanticAction(ReportFormatKind kind);
DateValue * relativeDateSemanticAction(DateValueKind kind);
DateValue * absoluteDateStringSemanticAction(const char * text);

EditField * editFieldDescriptionSemanticAction(const char * text);
EditField * editFieldDateValueSemanticAction(DateValue * dateValue);
EditField * editFieldCategorySemanticAction(const char * identifier);
EditField * editFieldAmountSemanticAction(const int amount);

EditFieldList * singletonEditFieldListSemanticAction(EditField * field);
EditFieldList * appendEditFieldListSemanticAction(EditFieldList * list, EditField * field);

PredefinedPeriod * predefinedPeriodSemanticAction(PredefinedPeriodKind kind);

DateFilter * dateFilterRangeSemanticAction(DateValue * fromDate, DateValue * toDate);
DateFilter * dateFilterPredefinedSemanticAction(PredefinedPeriod * period);

OptionalDescription * emptyOptionalDescriptionSemanticAction();
OptionalDescription * presentOptionalDescriptionSemanticAction(const char * text);
OptionalEndDate * emptyOptionalEndDateSemanticAction();
OptionalEndDate * presentOptionalEndDateSemanticAction(DateValue * dateValue);
OptionalStartDate * emptyOptionalStartDateSemanticAction();
OptionalStartDate * presentOptionalStartDateSemanticAction(DateValue * dateValue);
OptionalOperationDate * emptyOptionalOperationDateSemanticAction();
OptionalOperationDate * presentOptionalOperationDateSemanticAction(DateValue * dateValue);
OptionalCategory * emptyOptionalCategorySemanticAction();
OptionalCategory * presentOptionalCategorySemanticAction(const char * identifier);
OptionalInstallments * emptyOptionalInstallmentsSemanticAction();
OptionalInstallments * presentOptionalInstallmentsSemanticAction(const int count);

FinalizeStatement * finalizeStatementSemanticAction(const int operationId);
ReportStatement * reportStatementSemanticAction(ReportFormat * format, DateFilter * dateFilter);
DeleteStatement * deleteStatementSemanticAction(const int operationId);
EditStatement * editStatementSemanticAction(const int operationId, EditFieldList * fields);
QueryStatement * queryStatementSemanticAction(DateFilter * dateFilter);
SubscriptionStatement * subscriptionStatementSemanticAction(const int amount, PredefinedPeriod * period, OptionalCategory * optionalCategory, OptionalStartDate * optionalStartDate, OptionalEndDate * optionalEndDate, OptionalDescription * optionalDescription);
IncomeStatement * incomeStatementSemanticAction(const int amount, OptionalCategory * optionalCategory, OptionalOperationDate * optionalOperationDate, OptionalDescription * optionalDescription);
ExpenseStatement * expenseStatementSemanticAction(const int amount, OptionalInstallments * optionalInstallments, OptionalCategory * optionalCategory, OptionalOperationDate * optionalOperationDate, OptionalDescription * optionalDescription);
CurrencyStatement * currencyStatementSemanticAction(const char * identifier);

Statement * wrapFinalizeStatementSemanticAction(FinalizeStatement * finalizeStmt);
Statement * wrapReportStatementSemanticAction(ReportStatement * reportStmt);
Statement * wrapDeleteStatementSemanticAction(DeleteStatement * deleteStmt);
Statement * wrapEditStatementSemanticAction(EditStatement * editStmt);
Statement * wrapQueryStatementSemanticAction(QueryStatement * queryStmt);
Statement * wrapSubscriptionStatementSemanticAction(SubscriptionStatement * subscriptionStmt);
Statement * wrapIncomeStatementSemanticAction(IncomeStatement * incomeStmt);
Statement * wrapExpenseStatementSemanticAction(ExpenseStatement * expenseStmt);
Statement * wrapCurrencyStatementSemanticAction(CurrencyStatement * currencyStmt);

StatementList * singletonStatementListSemanticAction(Statement * statement);
StatementList * appendStatementListSemanticAction(StatementList * list, Statement * statement);

Program * buildProgramSemanticAction(StatementList * statements);

#endif
