#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
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

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

ReportFormat * ReportFormatSemanticAction(ReportFormatKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ReportFormat * format = calloc(1, sizeof(ReportFormat));
	format->kind = kind;
	return format;
}

Date * RelativeDateSemanticAction(DateKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Date * date = calloc(1, sizeof(Date));
	date->kind = kind;
	return date;
}

Date * StringDateSemanticAction(const char * string) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Date * date = calloc(1, sizeof(Date));
	date->kind = DATE_KIND_LITERAL;
	date->literal = (char *) string;
	return date;
}

EditField * DescriptionEditFieldSemanticAction(const char * string) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_DESCRIPTION;
	field->description = (char *) string;
	return field;
}

EditField * DateEditFieldSemanticAction(Date * date) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_DATE;
	field->date = date;
	return field;
}

EditField * CategoryEditFieldSemanticAction(const char * id) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_CATEGORY;
	field->categoryId = (char *) id;
	return field;
}

EditField * AmountEditFieldSemanticAction(const int amount) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditField * field = calloc(1, sizeof(EditField));
	field->kind = EDIT_FIELD_AMOUNT;
	field->amount = amount;
	return field;
}

EditFieldList * SingleEditFieldSemanticAction(EditField * field) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditFieldList * list = calloc(1, sizeof(EditFieldList));
	list->field = field;
	return list;
}

EditFieldList * ConstructEditFieldsSemanticAction(EditFieldList * tail, EditField * head) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditFieldList * list = calloc(1, sizeof(EditFieldList));
	list->field = head;
	list->next = tail;
	return list;
}

Frequency * FrequencySemanticAction(FrequencyKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Frequency * frequency = calloc(1, sizeof(Frequency));
	frequency->kind = kind;
	return frequency;
}

DatePeriod * DateRangePeriodSemanticAction(Date * fromDate, Date * toDate) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DatePeriod * period = calloc(1, sizeof(DatePeriod));
	period->kind = DATE_PERIOD_RANGE;
	period->fromDate = fromDate;
	period->toDate = toDate;
	return period;
}

DatePeriod * FrequencyDatePeriodSemanticAction(Frequency * frequency) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DatePeriod * period = calloc(1, sizeof(DatePeriod));
	period->kind = DATE_PERIOD_FREQUENCY;
	period->frequency = frequency;
	return period;
}

OptionalDescription * EmptyOptionalDescriptionSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalDescription * PresentOptionalDescriptionSemanticAction(const char * string) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalDescription * optional = calloc(1, sizeof(OptionalDescription));
	optional->text = (char *) string;
	return optional;
}

OptionalUntil * EmptyOptionalUntilSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalUntil * PresentOptionalUntilSemanticAction(Date * date) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalUntil * optional = calloc(1, sizeof(OptionalUntil));
	optional->date = date;
	return optional;
}

OptionalFrom * EmptyOptionalFromSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalFrom * PresentOptionalFromSemanticAction(Date * date) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalFrom * optional = calloc(1, sizeof(OptionalFrom));
	optional->date = date;
	return optional;
}

OptionalDate * EmptyOptionalDateSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalDate * PresentOptionalDateSemanticAction(Date * date) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalDate * optional = calloc(1, sizeof(OptionalDate));
	optional->date = date;
	return optional;
}

OptionalCategory * EmptyOptionalCategorySemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalCategory * PresentOptionalCategorySemanticAction(const char * id) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalCategory * optional = calloc(1, sizeof(OptionalCategory));
	optional->id = (char *) id;
	return optional;
}

OptionalInstallments * EmptyOptionalInstallmentsSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalInstallments * PresentOptionalInstallmentsSemanticAction(const int count) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalInstallments * optional = calloc(1, sizeof(OptionalInstallments));
	optional->count = count;
	return optional;
}

FinalizeSentence * FinalizeSentenceSemanticAction(const int number) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	FinalizeSentence * sentence = calloc(1, sizeof(FinalizeSentence));
	sentence->number = number;
	return sentence;
}

ReportSentence * ReportSentenceSemanticAction(ReportFormat * format, DatePeriod * period) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ReportSentence * sentence = calloc(1, sizeof(ReportSentence));
	sentence->format = format;
	sentence->period = period;
	return sentence;
}

DeleteSentence * DeleteSentenceSemanticAction(const int number) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DeleteSentence * sentence = calloc(1, sizeof(DeleteSentence));
	sentence->number = number;
	return sentence;
}

EditSentence * EditSentenceSemanticAction(const int number, EditFieldList * fields) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditSentence * sentence = calloc(1, sizeof(EditSentence));
	sentence->number = number;
	sentence->fields = fields;
	return sentence;
}

QuerySentence * QuerySentenceSemanticAction(DatePeriod * period) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	QuerySentence * sentence = calloc(1, sizeof(QuerySentence));
	sentence->period = period;
	return sentence;
}

SubscriptionSentence * SubscriptionSentenceSemanticAction(const int number, Frequency * frequency, OptionalCategory * optionalCategory, OptionalFrom * optionalFrom, OptionalUntil * optionalUntil, OptionalDescription * optionalDescription) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	SubscriptionSentence * sentence = calloc(1, sizeof(SubscriptionSentence));
	sentence->number = number;
	sentence->frequency = frequency;
	sentence->optionalCategory = optionalCategory;
	sentence->optionalFrom = optionalFrom;
	sentence->optionalUntil = optionalUntil;
	sentence->optionalDescription = optionalDescription;
	return sentence;
}

IncomeSentence * IncomeSentenceSemanticAction(const int number, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IncomeSentence * sentence = calloc(1, sizeof(IncomeSentence));
	sentence->number = number;
	sentence->optionalCategory = optionalCategory;
	sentence->optionalDate = optionalDate;
	sentence->optionalDescription = optionalDescription;
	return sentence;
}

ExpenseSentence * ExpenseSentenceSemanticAction(const int number, OptionalInstallments * optionalInstallments, OptionalCategory * optionalCategory, OptionalDate * optionalDate, OptionalDescription * optionalDescription) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExpenseSentence * sentence = calloc(1, sizeof(ExpenseSentence));
	sentence->number = number;
	sentence->optionalInstallments = optionalInstallments;
	sentence->optionalCategory = optionalCategory;
	sentence->optionalDate = optionalDate;
	sentence->optionalDescription = optionalDescription;
	return sentence;
}

CurrencySentence * CurrencySentenceSemanticAction(const char * id) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CurrencySentence * sentence = calloc(1, sizeof(CurrencySentence));
	sentence->id = (char *) id;
	return sentence;
}

Sentence * SentenceFromFinalizeSemanticAction(FinalizeSentence * finalizeSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_FINALIZE;
	sentence->finalizeSentence = finalizeSentence;
	return sentence;
}

Sentence * SentenceFromReportSemanticAction(ReportSentence * reportSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_REPORT;
	sentence->reportSentence = reportSentence;
	return sentence;
}

Sentence * SentenceFromDeleteSemanticAction(DeleteSentence * deleteSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_DELETE;
	sentence->deleteSentence = deleteSentence;
	return sentence;
}

Sentence * SentenceFromEditSemanticAction(EditSentence * editSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_EDIT;
	sentence->editSentence = editSentence;
	return sentence;
}

Sentence * SentenceFromQuerySemanticAction(QuerySentence * querySentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_QUERY;
	sentence->querySentence = querySentence;
	return sentence;
}

Sentence * SentenceFromSubscriptionSemanticAction(SubscriptionSentence * subscriptionSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_SUBSCRIPTION;
	sentence->subscriptionSentence = subscriptionSentence;
	return sentence;
}

Sentence * SentenceFromIncomeSemanticAction(IncomeSentence * incomeSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_INCOME;
	sentence->incomeSentence = incomeSentence;
	return sentence;
}

Sentence * SentenceFromExpenseSemanticAction(ExpenseSentence * expenseSentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_EXPENSE;
	sentence->expenseSentence = expenseSentence;
	return sentence;
}

Sentence * SentenceFromCurrencySemanticAction(CurrencySentence * currencySentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->kind = SENTENCE_CURRENCY;
	sentence->currencySentence = currencySentence;
	return sentence;
}

Sentences * SentenceSemanticAction(Sentence * sentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentences * sentences = calloc(1, sizeof(Sentences));
	sentences->sentence = sentence;
	sentences->next = NULL;
	return sentences;
}

Sentences * SentencesSentenceSemanticAction(Sentences * sentences, Sentence * sentence) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentences * newSentences = calloc(1, sizeof(Sentences));
	newSentences->sentence = sentence;
	newSentences->next = sentences;
	return newSentences;
}

Program * SentencesProgramSemanticAction(Sentences * sentences) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->sentences = sentences;
	_compilerState->abstractSyntaxTree = program;
	return program;
}
