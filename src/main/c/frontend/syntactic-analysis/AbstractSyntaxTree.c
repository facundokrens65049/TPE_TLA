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

void destroyDate(Date * date) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (date != NULL) {
		if (date->kind == DATE_KIND_LITERAL) {
			free(date->literal);
		}
		free(date);
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
			case EDIT_FIELD_DATE:
				destroyDate(field->date);
				break;
			case EDIT_FIELD_DESCRIPTION:
				free(field->description);
				break;
		}
		free(field);
	}
}

void destroyEditFieldList(EditFieldList * list) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	// Iterativo en vez de recursivo: una lista larga no debe consumir stack.
	while (list != NULL) {
		EditFieldList * next = list->next;
		destroyEditField(list->field);
		free(list);
		list = next;
	}
}

void destroyFrequency(Frequency * frequency) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (frequency != NULL) {
		free(frequency);
	}
}

void destroyDatePeriod(DatePeriod * period) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (period != NULL) {
		switch (period->kind) {
			case DATE_PERIOD_RANGE:
				destroyDate(period->fromDate);
				destroyDate(period->toDate);
				break;
			case DATE_PERIOD_FREQUENCY:
				destroyFrequency(period->frequency);
				break;
		}
		free(period);
	}
}

void destroyOptionalDescription(OptionalDescription * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional->text);
		free(optional);
	}
}

void destroyOptionalUntil(OptionalUntil * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDate(optional->date);
		free(optional);
	}
}

void destroyOptionalFrom(OptionalFrom * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDate(optional->date);
		free(optional);
	}
}

void destroyOptionalDate(OptionalDate * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		destroyDate(optional->date);
		free(optional);
	}
}

void destroyOptionalCategory(OptionalCategory * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional->id);
		free(optional);
	}
}

void destroyOptionalInstallments(OptionalInstallments * optional) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optional != NULL) {
		free(optional);
	}
}

void destroyFinalizeSentence(FinalizeSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		free(sentence);
	}
}

void destroyReportSentence(ReportSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyReportFormat(sentence->format);
		destroyDatePeriod(sentence->period);
		free(sentence);
	}
}

void destroyDeleteSentence(DeleteSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		free(sentence);
	}
}

void destroyEditSentence(EditSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyEditFieldList(sentence->fields);
		free(sentence);
	}
}

void destroyQuerySentence(QuerySentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyDatePeriod(sentence->period);
		free(sentence);
	}
}

void destroySubscriptionSentence(SubscriptionSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyFrequency(sentence->frequency);
		destroyOptionalCategory(sentence->optionalCategory);
		destroyOptionalFrom(sentence->optionalFrom);
		destroyOptionalUntil(sentence->optionalUntil);
		destroyOptionalDescription(sentence->optionalDescription);
		free(sentence);
	}
}

void destroyIncomeSentence(IncomeSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyOptionalCategory(sentence->optionalCategory);
		destroyOptionalDate(sentence->optionalDate);
		destroyOptionalDescription(sentence->optionalDescription);
		free(sentence);
	}
}

void destroyExpenseSentence(ExpenseSentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		destroyOptionalInstallments(sentence->optionalInstallments);
		destroyOptionalCategory(sentence->optionalCategory);
		destroyOptionalDate(sentence->optionalDate);
		destroyOptionalDescription(sentence->optionalDescription);
		free(sentence);
	}
}

void destroyCurrencySentence(CurrencySentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		free(sentence->id);
		free(sentence);
	}
}

void destroySentence(Sentence * sentence) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
		switch (sentence->kind) {
			case SENTENCE_CURRENCY:
				destroyCurrencySentence(sentence->currencySentence);
				break;
			case SENTENCE_EXPENSE:
				destroyExpenseSentence(sentence->expenseSentence);
				break;
			case SENTENCE_INCOME:
				destroyIncomeSentence(sentence->incomeSentence);
				break;
			case SENTENCE_SUBSCRIPTION:
				destroySubscriptionSentence(sentence->subscriptionSentence);
				break;
			case SENTENCE_QUERY:
				destroyQuerySentence(sentence->querySentence);
				break;
			case SENTENCE_EDIT:
				destroyEditSentence(sentence->editSentence);
				break;
			case SENTENCE_DELETE:
				destroyDeleteSentence(sentence->deleteSentence);
				break;
			case SENTENCE_REPORT:
				destroyReportSentence(sentence->reportSentence);
				break;
			case SENTENCE_FINALIZE:
				destroyFinalizeSentence(sentence->finalizeSentence);
				break;
		}
		free(sentence);
	}
}

void destroySentences(Sentences * sentences) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	// Iterativo en vez de recursivo: un programa con muchas sentencias no debe
	// consumir un marco de stack por cada una.
	while (sentences != NULL) {
		Sentences * next = sentences->next;
		destroySentence(sentences->sentence);
		free(sentences);
		sentences = next;
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroySentences(program->sentences);
		free(program);
	}
}
