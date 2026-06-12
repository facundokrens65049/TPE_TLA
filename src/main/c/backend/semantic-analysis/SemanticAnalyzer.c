#include "SemanticAnalyzer.h"
#include "../../support/language/DateUtils.h"
#include "../../support/language/String.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

// La tabla de simbolos la maneja este modulo (para poder liberarla en el
// shutdown, que no recibe estado) y ademas se publica en el CompilerState.
static SymbolTable * _symbolTable = NULL;

#define DEFAULT_CURRENCY "ARS"

static void _destroySymbolTable(SymbolTable * table);

/** Shutdown module's internal state. */
void _shutdownSemanticAnalyzerModule() {
	if (_symbolTable != NULL) {
		_destroySymbolTable(_symbolTable);
		_symbolTable = NULL;
	}
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule() {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownSemanticAnalyzerModule;
}

/* SYMBOL TABLE */

static char * _duplicateString(const char * source) {
	const size_t size = strlen(source) + 1;
	char * copy = malloc(size);
	memcpy(copy, source, size);
	return copy;
}

static SymbolTable * _createSymbolTable(void) {
	SymbolTable * table = calloc(1, sizeof(SymbolTable));
	table->activeCurrency = _duplicateString(DEFAULT_CURRENCY);
	return table;
}

static void _destroySymbolTable(SymbolTable * table) {
	if (table == NULL) {
		return;
	}
	free(table->activeCurrency);
	for (size_t i = 0; i < table->categoryCount; ++i) {
		free(table->categories[i]);
	}
	free(table->categories);
	free(table);
}

static void _setActiveCurrency(SymbolTable * table, const char * id) {
	free(table->activeCurrency);
	table->activeCurrency = _duplicateString(id);
}

// Agrega una categoria normalizada al set, salteando las que ya estan.
static void _registerCategory(SymbolTable * table, const char * rawId) {
	char * normalized = normalizeCategory(rawId);
	for (size_t i = 0; i < table->categoryCount; ++i) {
		if (strcmp(table->categories[i], normalized) == 0) {
			free(normalized);
			return;
		}
	}
	if (table->categoryCount == table->categoryCapacity) {
		const size_t capacity = (table->categoryCapacity == 0) ? 4 : table->categoryCapacity * 2;
		table->categories = realloc(table->categories, capacity * sizeof(char *));
		table->categoryCapacity = capacity;
	}
	table->categories[table->categoryCount++] = normalized;
}

/* SEMANTIC RULES (helpers) */

// Lleva cualquier fecha (literal o relativa) a un entero YYYYMMDD comparable.
static DateValue _resolveDate(const Date * date) {
	switch (date->kind) {
		case DATE_KIND_LITERAL: return parseLiteralDate(date->literal);
		case DATE_KIND_TODAY: return today();
		case DATE_KIND_YESTERDAY: return yesterday();
		case DATE_KIND_TOMORROW: return tomorrow();
		default: return INVALID_DATE_VALUE;
	}
}

// Toda fecha literal tiene que ser una fecha real. Las relativas (hoy/ayer/
// maniana) y un date NULL (slot opcional ausente) son siempre validos.
static bool _checkLiteralDate(const Date * date) {
	if (date == NULL || date->kind != DATE_KIND_LITERAL) {
		return true;
	}
	if (parseLiteralDate(date->literal) == INVALID_DATE_VALUE) {
		logError(_logger, "Invalid calendar date literal: \"%s\".", date->literal);
		return false;
	}
	return true;
}

// Los montos viajan en centavos: imprimimos el numero "humano" cuando
// reportamos.
static void _formatCentavos(long long centavos, char * buffer, size_t size) {
	const long long whole = centavos / 100;
	const long long cents = (centavos % 100 + 100) % 100;
	snprintf(buffer, size, "%lld.%02lld", whole, cents);
}

static bool _checkPositiveAmount(const long long amount, const char * what) {
	if (amount <= 0) {
		char buffer[32];
		_formatCentavos(amount, buffer, sizeof(buffer));
		logError(_logger, "The %s amount must be greater than 0, but it is %s.", what, buffer);
		return false;
	}
	return true;
}

// Los campos que conceptualmente son enteros (ids de operacion, cantidad de
// cuotas) viajan tambien en centavos despues del cambio a decimales. Para
// volver al entero original tienen que ser un múltiplo de 100.
static bool _checkWholeInteger(const long long centavos, const char * what) {
	if (centavos % 100 != 0) {
		char buffer[32];
		_formatCentavos(centavos, buffer, sizeof(buffer));
		logError(_logger, "The %s must be an integer, but it is %s.", what, buffer);
		return false;
	}
	return true;
}

// Valida los dos extremos del rango como literales, los resuelve y exige
// desde <= hasta (un mismo dia es un rango valido). "context" nombra el error.
static bool _checkDateRange(const Date * fromDate, const Date * toDate, const char * context) {
	if (!_checkLiteralDate(fromDate) || !_checkLiteralDate(toDate)) {
		return false;
	}
	const DateValue from = _resolveDate(fromDate);
	const DateValue to = _resolveDate(toDate);
	if (from == INVALID_DATE_VALUE || to == INVALID_DATE_VALUE) {
		logError(_logger, "Could not resolve the dates of the %s.", context);
		return false;
	}
	if (from > to) {
		logError(_logger, "The %s is inverted: start %d is after end %d.", context, from, to);
		return false;
	}
	return true;
}

/* PRIVATE FUNCTIONS */

static CompilationStatus _analyzeSentence(Sentence * sentence, SymbolTable * table);

static CompilationStatus _analyzeSentence(Sentence * sentence, SymbolTable * table) {
	switch (sentence->kind) {
		case SENTENCE_CURRENCY: {
			CurrencySentence * currency = sentence->currencySentence;
			_setActiveCurrency(table, currency->id);
			break;
		}
		case SENTENCE_EXPENSE: {
			ExpenseSentence * expense = sentence->expenseSentence;
			if (!_checkPositiveAmount(expense->number, "expense")) {
				return FAILED;
			}
			if (expense->optionalInstallments != NULL) {
				if (!_checkWholeInteger(expense->optionalInstallments->count, "installments count")) {
					return FAILED;
				}
				const long long installments = expense->optionalInstallments->count / 100;
				if (installments < 1) {
					logError(_logger, "The installments count must be at least 1, but it is %lld.", installments);
					return FAILED;
				}
			}
			if (expense->optionalDate != NULL && !_checkLiteralDate(expense->optionalDate->date)) {
				return FAILED;
			}
			if (expense->optionalCategory != NULL) {
				_registerCategory(table, expense->optionalCategory->id);
			}
			break;
		}
		case SENTENCE_INCOME: {
			IncomeSentence * income = sentence->incomeSentence;
			if (!_checkPositiveAmount(income->number, "income")) {
				return FAILED;
			}
			if (income->optionalDate != NULL && !_checkLiteralDate(income->optionalDate->date)) {
				return FAILED;
			}
			if (income->optionalCategory != NULL) {
				_registerCategory(table, income->optionalCategory->id);
			}
			break;
		}
		case SENTENCE_SUBSCRIPTION: {
			SubscriptionSentence * subscription = sentence->subscriptionSentence;
			if (!_checkPositiveAmount(subscription->number, "subscription")) {
				return FAILED;
			}
			Date * from = (subscription->optionalFrom != NULL) ? subscription->optionalFrom->date : NULL;
			Date * until = (subscription->optionalUntil != NULL) ? subscription->optionalUntil->date : NULL;
			if (!_checkLiteralDate(from) || !_checkLiteralDate(until)) {
				return FAILED;
			}
			// si estan los dos extremos, hasta >= desde
			if (from != NULL && until != NULL && !_checkDateRange(from, until, "subscription period")) {
				return FAILED;
			}
			if (subscription->optionalCategory != NULL) {
				_registerCategory(table, subscription->optionalCategory->id);
			}
			break;
		}
		case SENTENCE_QUERY: {
			DatePeriod * period = sentence->querySentence->period;
			if (period->kind == DATE_PERIOD_RANGE
				&& !_checkDateRange(period->fromDate, period->toDate, "query range")) {
				return FAILED;
			}
			break;
		}
		case SENTENCE_EDIT: {
			EditSentence * edit = sentence->editSentence;
			if (!_checkWholeInteger(edit->number, "operation id")) {
				return FAILED;
			}
			for (EditFieldList * node = edit->fields; node != NULL; node = node->next) {
				EditField * field = node->field;
				if (field->kind == EDIT_FIELD_DATE && !_checkLiteralDate(field->date)) {
					return FAILED;
				}
				if (field->kind == EDIT_FIELD_AMOUNT && !_checkPositiveAmount(field->amount, "edit amount")) {
					return FAILED;
				}
				if (field->kind == EDIT_FIELD_CATEGORY) {
					_registerCategory(table, field->categoryId);
				}
			}
			break;
		}
		case SENTENCE_DELETE:
			// la existencia del id se chequea en runtime contra la DB
			if (!_checkWholeInteger(sentence->deleteSentence->number, "operation id")) {
				return FAILED;
			}
			break;
		case SENTENCE_REPORT: {
			DatePeriod * period = sentence->reportSentence->period;
			if (period->kind == DATE_PERIOD_RANGE
				&& !_checkDateRange(period->fromDate, period->toDate, "report range")) {
				return FAILED;
			}
			break;
		}
		case SENTENCE_FINALIZE:
			// existencia del id y compatibilidad de tipo: chequeos de runtime (DB)
			if (!_checkWholeInteger(sentence->finalizeSentence->number, "operation id")) {
				return FAILED;
			}
			break;
		default:
			logError(_logger, "The specified sentence kind is unknown: %d", sentence->kind);
			return FAILED;
	}
	return SUCCEEDED;
}

/* PUBLIC FUNCTIONS */

CompilationStatus executeSemanticAnalysis(CompilerState * compilerState) {
	logDebugging(_logger, "Executing semantic analysis...");
	Program * program = compilerState->abstractSyntaxTree;
	if (program == NULL) {
		logError(_logger, "The program has no AST to analyze.");
		return FAILED;
	}
	_symbolTable = _createSymbolTable();
	compilerState->symbolTable = _symbolTable;
	CompilationStatus status = SUCCEEDED;
	for (Sentences * sentences = program->sentences; sentences != NULL; sentences = sentences->next) {
		if (_analyzeSentence(sentences->sentence, _symbolTable) != SUCCEEDED) {
			status = FAILED;
		}
	}
	logDebugging(_logger, "Semantic analysis is done (status: %d).", status);
	return status;
}
