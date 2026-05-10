#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */

typedef enum ReportFormatKind ReportFormatKind;
typedef enum DateKind DateKind;
typedef enum FrequencyKind FrequencyKind;
typedef enum DatePeriodKind DatePeriodKind;
typedef enum EditFieldKind EditFieldKind;
typedef enum SentenceKind SentenceKind;

typedef struct ReportFormat ReportFormat;
typedef struct Date Date;
typedef struct EditField EditField;
typedef struct EditFieldList EditFieldList;
typedef struct Frequency Frequency;
typedef struct DatePeriod DatePeriod;
typedef struct OptionalDescription OptionalDescription;
typedef struct OptionalUntil OptionalUntil;
typedef struct OptionalFrom OptionalFrom;
typedef struct OptionalDate OptionalDate;
typedef struct OptionalCategory OptionalCategory;
typedef struct OptionalInstallments OptionalInstallments;
typedef struct FinalizeSentence FinalizeSentence;
typedef struct ReportSentence ReportSentence;
typedef struct DeleteSentence DeleteSentence;
typedef struct EditSentence EditSentence;
typedef struct QuerySentence QuerySentence;
typedef struct SubscriptionSentence SubscriptionSentence;
typedef struct IncomeSentence IncomeSentence;
typedef struct ExpenseSentence ExpenseSentence;
typedef struct CurrencySentence CurrencySentence;
typedef struct Sentence Sentence;
typedef struct Sentences Sentences;
typedef struct Program Program;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */

enum ReportFormatKind {
	REPORT_FORMAT_HTML,
	REPORT_FORMAT_PLAIN_TEXT,
	REPORT_FORMAT_PDF
};

enum DateKind {
	DATE_KIND_LITERAL,
	DATE_KIND_TODAY,
	DATE_KIND_YESTERDAY,
	DATE_KIND_TOMORROW
};

enum FrequencyKind {
	FREQUENCY_MONTHLY,
	FREQUENCY_WEEKLY,
	FREQUENCY_YEARLY
};

enum DatePeriodKind {
	DATE_PERIOD_RANGE,
	DATE_PERIOD_FREQUENCY
};

enum EditFieldKind {
	EDIT_FIELD_AMOUNT,
	EDIT_FIELD_CATEGORY,
	EDIT_FIELD_DATE,
	EDIT_FIELD_DESCRIPTION
};

enum SentenceKind {
	SENTENCE_CURRENCY,
	SENTENCE_EXPENSE,
	SENTENCE_INCOME,
	SENTENCE_SUBSCRIPTION,
	SENTENCE_QUERY,
	SENTENCE_EDIT,
	SENTENCE_DELETE,
	SENTENCE_REPORT,
	SENTENCE_FINALIZE
};

struct ReportFormat {
	ReportFormatKind kind;
};

struct Date {
	DateKind kind;
	union {
		char * literal;
	};
};

struct EditField {
	EditFieldKind kind;
	union {
		int amount;
		char * categoryId;
		Date * date;
		char * description;
	};
};

struct EditFieldList {
	EditField * field;
	EditFieldList * next;
};

struct Frequency {
	FrequencyKind kind;
};

struct DatePeriod {
	DatePeriodKind kind;
	union {
		struct {
			Date * fromDate;
			Date * toDate;
		};
		Frequency * frequency;
	};
};

struct OptionalDescription {
	char * text;
};

struct OptionalUntil {
	Date * date;
};

struct OptionalFrom {
	Date * date;
};

struct OptionalDate {
	Date * date;
};

struct OptionalCategory {
	char * id;
};

struct OptionalInstallments {
	int count;
};

struct FinalizeSentence {
	int number;
};

struct ReportSentence {
	ReportFormat * format;
	DatePeriod * period;
};

struct DeleteSentence {
	int number;
};

struct EditSentence {
	int number;
	EditFieldList * fields;
};

struct QuerySentence {
	DatePeriod * period;
};

struct SubscriptionSentence {
	int number;
	Frequency * frequency;
	OptionalCategory * optionalCategory;
	OptionalFrom * optionalFrom;
	OptionalUntil * optionalUntil;
	OptionalDescription * optionalDescription;
};

struct IncomeSentence {
	int number;
	OptionalCategory * optionalCategory;
	OptionalDate * optionalDate;
	OptionalDescription * optionalDescription;
};

struct ExpenseSentence {
	int number;
	OptionalInstallments * optionalInstallments;
	OptionalCategory * optionalCategory;
	OptionalDate * optionalDate;
	OptionalDescription * optionalDescription;
};

struct CurrencySentence {
	char * id;
};

struct Sentence {
	SentenceKind kind;
	union {
		CurrencySentence * currencySentence;
		ExpenseSentence * expenseSentence;
		IncomeSentence * incomeSentence;
		SubscriptionSentence * subscriptionSentence;
		QuerySentence * querySentence;
		EditSentence * editSentence;
		DeleteSentence * deleteSentence;
		ReportSentence * reportSentence;
		FinalizeSentence * finalizeSentence;
	};
};

struct Sentences {
	Sentence * sentence;
	Sentences * next; /* linked list */
};

struct Program {
	Sentences * sentences;
};

/**
 * Node recursive super-duper-trambolik-destructors.
 */

void destroyReportFormat(ReportFormat * format);
void destroyDate(Date * date);
void destroyEditField(EditField * field);
void destroyEditFieldList(EditFieldList * list);
void destroyFrequency(Frequency * frequency);
void destroyDatePeriod(DatePeriod * period);
void destroyOptionalDescription(OptionalDescription * optional);
void destroyOptionalUntil(OptionalUntil * optional);
void destroyOptionalFrom(OptionalFrom * optional);
void destroyOptionalDate(OptionalDate * optional);
void destroyOptionalCategory(OptionalCategory * optional);
void destroyOptionalInstallments(OptionalInstallments * optional);
void destroyFinalizeSentence(FinalizeSentence * sentence);
void destroyReportSentence(ReportSentence * sentence);
void destroyDeleteSentence(DeleteSentence * sentence);
void destroyEditSentence(EditSentence * sentence);
void destroyQuerySentence(QuerySentence * sentence);
void destroySubscriptionSentence(SubscriptionSentence * sentence);
void destroyIncomeSentence(IncomeSentence * sentence);
void destroyExpenseSentence(ExpenseSentence * sentence);
void destroyCurrencySentence(CurrencySentence * sentence);
void destroySentence(Sentence * sentence);
void destroySentences(Sentences * sentences);
void destroyProgram(Program * program);

#endif
