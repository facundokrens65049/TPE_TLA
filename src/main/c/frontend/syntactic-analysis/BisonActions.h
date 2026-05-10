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

ReportFormat * reportFormatSemanticAction(ReportFormatKind kind);

Date * relativeDateSemanticAction(DateKind kind);
Date * stringDateSemanticAction(const char * string);

EditField * descriptionEditFieldSemanticAction(const char * string);
EditField * dateEditFieldSemanticAction(Date * date);
EditField * categoryEditFieldSemanticAction(const char * id);
EditField * amountEditFieldSemanticAction(const int amount);

EditFieldList * singleEditFieldSemanticAction(EditField * field);
EditFieldList * consEditFieldsSemanticAction(EditFieldList * tail, EditField * head);

Frequency * frequencySemanticAction(FrequencyKind kind);

DatePeriod * dateRangePeriodSemanticAction(Date * fromDate, Date * toDate);
DatePeriod * frequencyDatePeriodSemanticAction(Frequency * frequency);

OptionalDescription * emptyOptionalDescriptionSemanticAction();
OptionalDescription * presentOptionalDescriptionSemanticAction(const char * string);
OptionalUntil * emptyOptionalUntilSemanticAction();
OptionalUntil * presentOptionalUntilSemanticAction(Date * date);
OptionalFrom * emptyOptionalFromSemanticAction();
OptionalFrom * presentOptionalFromSemanticAction(Date * date);
OptionalDate * emptyOptionalDateSemanticAction();
OptionalDate * presentOptionalDateSemanticAction(Date * date);
OptionalCategory * emptyOptionalCategorySemanticAction();
OptionalCategory * presentOptionalCategorySemanticAction(const char * id);
OptionalInstallments * emptyOptionalInstallmentsSemanticAction();
OptionalInstallments * presentOptionalInstallmentsSemanticAction(const int count);

FinalizeSentence * finalizeSentenceSemanticAction(const int number);
ReportSentence * reportSentenceSemanticAction(ReportFormat * format, DatePeriod * period);
DeleteSentence * deleteSentenceSemanticAction(const int number);
EditSentence * editSentenceSemanticAction(const int number, EditFieldList * fields);
QuerySentence * querySentenceSemanticAction(DatePeriod * period);
SubscriptionSentence * subscriptionSentenceSemanticAction(const int number, Frequency * frequency, OptionalCategory * optionalCategory, OptionalFrom * optionalFrom, OptionalUntil * optionalUntil, OptionalDescription * optionalDescription);
IncomeSentence * incomeSentenceSemanticAction(const int number, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription);
ExpenseSentence * expenseSentenceSemanticAction(const int number, OptionalInstallments * optionalInstallments, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription);
CurrencySentence * currencySentenceSemanticAction(const char * id);

Sentence * sentenceFromFinalizeSemanticAction(FinalizeSentence * finalizeSentence);
Sentence * sentenceFromReportSemanticAction(ReportSentence * reportSentence);
Sentence * sentenceFromDeleteSemanticAction(DeleteSentence * deleteSentence);
Sentence * sentenceFromEditSemanticAction(EditSentence * editSentence);
Sentence * sentenceFromQuerySemanticAction(QuerySentence * querySentence);
Sentence * sentenceFromSubscriptionSemanticAction(SubscriptionSentence * subscriptionSentence);
Sentence * sentenceFromIncomeSemanticAction(IncomeSentence * incomeSentence);
Sentence * sentenceFromExpenseSemanticAction(ExpenseSentence * expenseSentence);
Sentence * sentenceFromCurrencySemanticAction(CurrencySentence * currencySentence);

Sentences * sentenceSemanticAction(Sentence * sentence);
Sentences * sentencesSentenceSemanticAction(Sentences * sentences, Sentence * sentence);

Program * sentencesProgramSemanticAction(Sentences * sentences);

#endif
