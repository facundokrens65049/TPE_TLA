#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions.
 */

ReportFormat * ReportFormatSemanticAction(ReportFormatKind kind);

Date * RelativeDateSemanticAction(DateKind kind);
Date * StringDateSemanticAction(const char * string);

EditField * DescriptionEditFieldSemanticAction(const char * string);
EditField * DateEditFieldSemanticAction(Date * date);
EditField * CategoryEditFieldSemanticAction(const char * id);
EditField * AmountEditFieldSemanticAction(const int amount);

EditFieldList * SingleEditFieldSemanticAction(EditField * field);
EditFieldList * ConstructEditFieldsSemanticAction(EditFieldList * tail, EditField * head);

Frequency * FrequencySemanticAction(FrequencyKind kind);

DatePeriod * DateRangePeriodSemanticAction(Date * fromDate, Date * toDate);
DatePeriod * FrequencyDatePeriodSemanticAction(Frequency * frequency);

OptionalDescription * EmptyOptionalDescriptionSemanticAction();
OptionalDescription * PresentOptionalDescriptionSemanticAction(const char * string);
OptionalUntil * EmptyOptionalUntilSemanticAction();
OptionalUntil * PresentOptionalUntilSemanticAction(Date * date);
OptionalFrom * EmptyOptionalFromSemanticAction();
OptionalFrom * PresentOptionalFromSemanticAction(Date * date);
OptionalDate * EmptyOptionalDateSemanticAction();
OptionalDate * PresentOptionalDateSemanticAction(Date * date);
OptionalCategory * EmptyOptionalCategorySemanticAction();
OptionalCategory * PresentOptionalCategorySemanticAction(const char * id);
OptionalInstallments * EmptyOptionalInstallmentsSemanticAction();
OptionalInstallments * PresentOptionalInstallmentsSemanticAction(const int count);

FinalizeSentence * FinalizeSentenceSemanticAction(const int number);
ReportSentence * ReportSentenceSemanticAction(ReportFormat * format, DatePeriod * period);
DeleteSentence * DeleteSentenceSemanticAction(const int number);
EditSentence * EditSentenceSemanticAction(const int number, EditFieldList * fields);
QuerySentence * QuerySentenceSemanticAction(DatePeriod * period);
SubscriptionSentence * SubscriptionSentenceSemanticAction(const int number, Frequency * frequency, OptionalCategory * optionalCategory, OptionalFrom * optionalFrom, OptionalUntil * optionalUntil, OptionalDescription * optionalDescription);
IncomeSentence * IncomeSentenceSemanticAction(const int number, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription);
ExpenseSentence * ExpenseSentenceSemanticAction(const int number, OptionalInstallments * optionalInstallments, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription);
CurrencySentence * CurrencySentenceSemanticAction(const char * id);

Sentence * SentenceFromFinalizeSemanticAction(FinalizeSentence * finalizeSentence);
Sentence * SentenceFromReportSemanticAction(ReportSentence * reportSentence);
Sentence * SentenceFromDeleteSemanticAction(DeleteSentence * deleteSentence);
Sentence * SentenceFromEditSemanticAction(EditSentence * editSentence);
Sentence * SentenceFromQuerySemanticAction(QuerySentence * querySentence);
Sentence * SentenceFromSubscriptionSemanticAction(SubscriptionSentence * subscriptionSentence);
Sentence * SentenceFromIncomeSemanticAction(IncomeSentence * incomeSentence);
Sentence * SentenceFromExpenseSemanticAction(ExpenseSentence * expenseSentence);
Sentence * SentenceFromCurrencySemanticAction(CurrencySentence * currencySentence);

Sentences * SentenceSemanticAction(Sentence * sentence);
Sentences * SentencesSentenceSemanticAction(Sentences * sentences, Sentence * sentence);

Program * SentencesProgramSemanticAction(Sentences * sentences);

#endif
