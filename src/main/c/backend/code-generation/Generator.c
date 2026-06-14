#include "Generator.h"
#include "reports/HtmlReport.h"
#include "reports/PdfReport.h"
#include "reports/ReportModel.h"
#include "reports/TextReport.h"
#include "../../support/io/EmitSql.h"
#include "../../support/language/DateUtils.h"
#include "../../support/language/String.h"
#include <stdbool.h>
#include <stdlib.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

// Columns of an operation, shared by queries (consultar) and by the flat
// projection that every INSERT/UPDATE sentence uses.
#define OPERATION_COLUMNS "id, tipo, monto, divisa, categoria, fecha, descripcion, estado"

/* PRIVATE FUNCTIONS */

// Amounts travel in cents as long long; in the SQL they are emitted with two
// decimals to fit into NUMERIC(15,2). Ids/installments also travel in cents
// but semantic analysis already guaranteed their fractional part is 0, so
// "cents / 100" reconstructs the original integer without loss.
static void _emitAmount(long long cents) {
	const long long whole = cents / 100;
	const long long fractional = (cents % 100 + 100) % 100;
	emitSql("%lld.%02lld", whole, fractional);
}

static long long _wholeFromCents(long long cents) {
	return cents / 100;
}

// Escapes a string to embed it as a single-quoted SQL literal (each ' is
// doubled). Returns heap-allocated memory the caller must free; does not add
// the surrounding quotes.
static char * _sqlEscape(const char * raw) {
	size_t quotes = 0;
	for (size_t i = 0; raw[i] != '\0'; ++i) {
		if (raw[i] == '\'') {
			++quotes;
		}
	}
	const size_t length = strlen(raw);
	char * escaped = malloc(length + quotes + 1);
	size_t out = 0;
	for (size_t i = 0; i < length; ++i) {
		escaped[out++] = raw[i];
		if (raw[i] == '\'') {
			escaped[out++] = '\'';
		}
	}
	escaped[out] = '\0';
	return escaped;
}

// Normalizes the category (reusing the symbol-table logic) and escapes it.
static char * _categoryLiteral(const char * raw) {
	char * normalized = normalizeCategory(raw);
	char * escaped = _sqlEscape(normalized);
	free(normalized);
	return escaped;
}

static DateValue _resolveDate(const Date * date) {
	switch (date->kind) {
		case DATE_KIND_LITERAL: return parseLiteralDate(date->literal);
		case DATE_KIND_TODAY: return today();
		case DATE_KIND_YESTERDAY: return yesterday();
		case DATE_KIND_TOMORROW: return tomorrow();
		default: return INVALID_DATE_VALUE;
	}
}

// Base date of an operation: its own if present, otherwise today. Writes the
// ISO format into "buffer" (>= 11 bytes) and returns the YYYYMMDD value so
// callers can keep operating on it.
static DateValue _baseDate(const Date * date, char * buffer) {
	const DateValue value = (date != NULL) ? _resolveDate(date) : today();
	formatDateValueIso(value, buffer);
	return value;
}

/* DDL */

// Idempotent relational schema. "operaciones" is the anchor table (its serial
// id is what editar/eliminar/finalizar touch at runtime); cuotas and
// suscripciones hang off it via FK.
static void _generatePrologue(void) {
	emitSql("-- =====================================================================\n");
	emitSql("-- Script SQL generado por el compilador del DSL de finanzas (PostgreSQL)\n");
	emitSql("-- =====================================================================\n\n");
	emitSql("-- Esquema relacional (idempotente).\n");
	emitSql(
		"CREATE TABLE IF NOT EXISTS operaciones (\n"
		"    id          SERIAL PRIMARY KEY,\n"
		"    tipo        VARCHAR(20)  NOT NULL,\n"
		"    monto       NUMERIC(15,2) NOT NULL,\n"
		"    divisa      VARCHAR(10)  NOT NULL,\n"
		"    categoria   VARCHAR(64),\n"
		"    fecha       DATE         NOT NULL,\n"
		"    descripcion VARCHAR(80),\n"
		"    estado      VARCHAR(20)  NOT NULL DEFAULT 'activo'\n"
		");\n\n");
	emitSql(
		"CREATE TABLE IF NOT EXISTS cuotas (\n"
		"    id            SERIAL PRIMARY KEY,\n"
		"    operacion_id  INTEGER NOT NULL REFERENCES operaciones(id) ON DELETE CASCADE,\n"
		"    numero_cuota  INTEGER NOT NULL,\n"
		"    total_cuotas  INTEGER NOT NULL,\n"
		"    monto         NUMERIC(15,2) NOT NULL,\n"
		"    divisa        VARCHAR(10)  NOT NULL,\n"
		"    fecha         DATE         NOT NULL,\n"
		"    estado        VARCHAR(20)  NOT NULL DEFAULT 'pendiente'\n"
		");\n\n");
	emitSql(
		"CREATE TABLE IF NOT EXISTS suscripciones (\n"
		"    id            SERIAL PRIMARY KEY,\n"
		"    operacion_id  INTEGER NOT NULL REFERENCES operaciones(id) ON DELETE CASCADE,\n"
		"    monto         NUMERIC(15,2) NOT NULL,\n"
		"    divisa        VARCHAR(10)  NOT NULL,\n"
		"    categoria     VARCHAR(64),\n"
		"    frecuencia    VARCHAR(20)  NOT NULL,\n"
		"    desde         DATE         NOT NULL,\n"
		"    hasta         DATE,\n"
		"    descripcion   VARCHAR(80),\n"
		"    estado        VARCHAR(20)  NOT NULL DEFAULT 'activo'\n"
		");\n\n");
	emitSql("-- Sentencias del programa.\n");
}

/* PER-SENTENCE GENERATION */

static const char * _frequencyName(const Frequency * frequency) {
	switch (frequency->kind) {
		case FREQUENCY_MONTHLY: return "mensual";
		case FREQUENCY_WEEKLY: return "semanal";
		case FREQUENCY_YEARLY: return "anual";
		default: return "mensual";
	}
}

// Optional category as SQL value: normalized quoted literal or NULL.
static void _emitCategoryValue(const OptionalCategory * category) {
	if (category == NULL) {
		emitSql("NULL");
		return;
	}
	char * literal = _categoryLiteral(category->id);
	emitSql("'%s'", literal);
	free(literal);
}

static void _emitDescriptionValue(const OptionalDescription * description) {
	if (description == NULL) {
		emitSql("NULL");
		return;
	}
	char * escaped = _sqlEscape(description->text);
	emitSql("'%s'", escaped);
	free(escaped);
}

static void _generateExpense(const ExpenseSentence * expense, const char * currency) {
	char base[ISO_DATE_BUFFER_SIZE];
	const Date * date = (expense->optionalDate != NULL) ? expense->optionalDate->date : NULL;
	_baseDate(date, base);

	if (expense->optionalInstallments == NULL) {
		// simple expense: a single row
		emitSql("INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
		emitSql("VALUES ('gasto', ");
		_emitAmount(expense->number);
		emitSql(", '%s', ", currency);
		_emitCategoryValue(expense->optionalCategory);
		emitSql(", DATE '%s', ", base);
		_emitDescriptionValue(expense->optionalDescription);
		emitSql(");\n\n");
		return;
	}

	// installment expense: the parent operation + N future obligations, one
	// per month from the base date, all carrying the parent's id (RETURNING).
	const long long count = _wholeFromCents(expense->optionalInstallments->count);
	emitSql("WITH nueva AS (\n");
	emitSql("    INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	emitSql("    VALUES ('cuotas', ");
	_emitAmount(expense->number);
	emitSql(", '%s', ", currency);
	_emitCategoryValue(expense->optionalCategory);
	emitSql(", DATE '%s', ", base);
	_emitDescriptionValue(expense->optionalDescription);
	emitSql(")\n");
	emitSql("    RETURNING id, fecha\n");
	emitSql(")\n");
	emitSql("INSERT INTO cuotas (operacion_id, numero_cuota, total_cuotas, monto, divisa, fecha)\n");
	emitSql("SELECT nueva.id, gs.n, %lld, ROUND(", count);
	_emitAmount(expense->number);
	emitSql("::numeric / %lld, 2), '%s',\n", count, currency);
	emitSql("       (nueva.fecha + ((gs.n - 1) * INTERVAL '1 month'))::date\n");
	emitSql("FROM nueva, generate_series(1, %lld) AS gs(n);\n\n", count);
}

static void _generateIncome(const IncomeSentence * income, const char * currency) {
	char base[ISO_DATE_BUFFER_SIZE];
	const Date * date = (income->optionalDate != NULL) ? income->optionalDate->date : NULL;
	_baseDate(date, base);

	emitSql("INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	emitSql("VALUES ('ingreso', ");
	_emitAmount(income->number);
	emitSql(", '%s', ", currency);
	_emitCategoryValue(income->optionalCategory);
	emitSql(", DATE '%s', ", base);
	_emitDescriptionValue(income->optionalDescription);
	emitSql(");\n\n");
}

static void _generateSubscription(const SubscriptionSentence * subscription, const char * currency) {
	char fromBuffer[ISO_DATE_BUFFER_SIZE];
	const Date * fromDate = (subscription->optionalFrom != NULL) ? subscription->optionalFrom->date : NULL;
	_baseDate(fromDate, fromBuffer);

	emitSql("WITH nueva AS (\n");
	emitSql("    INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	emitSql("    VALUES ('suscripcion', ");
	_emitAmount(subscription->number);
	emitSql(", '%s', ", currency);
	_emitCategoryValue(subscription->optionalCategory);
	emitSql(", DATE '%s', ", fromBuffer);
	_emitDescriptionValue(subscription->optionalDescription);
	emitSql(")\n");
	emitSql("    RETURNING id\n");
	emitSql(")\n");
	emitSql("INSERT INTO suscripciones (operacion_id, monto, divisa, categoria, frecuencia, desde, hasta, descripcion)\n");
	emitSql("SELECT nueva.id, ");
	_emitAmount(subscription->number);
	emitSql(", '%s', ", currency);
	_emitCategoryValue(subscription->optionalCategory);
	emitSql(", '%s', DATE '%s', ", _frequencyName(subscription->frequency), fromBuffer);
	if (subscription->optionalUntil != NULL) {
		char untilBuffer[ISO_DATE_BUFFER_SIZE];
		formatDateValueIso(_resolveDate(subscription->optionalUntil->date), untilBuffer);
		emitSql("DATE '%s', ", untilBuffer);
	}
	else {
		emitSql("NULL, ");
	}
	_emitDescriptionValue(subscription->optionalDescription);
	emitSql("\nFROM nueva;\n\n");
}

static void _generateEdit(const EditSentence * edit) {
	emitSql("UPDATE operaciones SET ");
	bool first = true;
	for (EditFieldList * node = edit->fields; node != NULL; node = node->next) {
		EditField * field = node->field;
		if (!first) {
			emitSql(", ");
		}
		first = false;
		switch (field->kind) {
			case EDIT_FIELD_AMOUNT:
				emitSql("monto = ");
				_emitAmount(field->amount);
				break;
			case EDIT_FIELD_CATEGORY: {
				char * literal = _categoryLiteral(field->categoryId);
				emitSql("categoria = '%s'", literal);
				free(literal);
				break;
			}
			case EDIT_FIELD_DATE: {
				char buffer[ISO_DATE_BUFFER_SIZE];
				formatDateValueIso(_resolveDate(field->date), buffer);
				emitSql("fecha = DATE '%s'", buffer);
				break;
			}
			case EDIT_FIELD_DESCRIPTION: {
				char * escaped = _sqlEscape(field->description);
				emitSql("descripcion = '%s'", escaped);
				free(escaped);
				break;
			}
			default:
				break;
		}
	}
	emitSql(" WHERE id = %lld;\n\n", _wholeFromCents(edit->number));
}

static void _generateDelete(const DeleteSentence * del) {
	emitSql("DELETE FROM operaciones WHERE id = %lld;\n\n", _wholeFromCents(del->number));
}

static void _generateFinalize(const FinalizeSentence * finalize) {
	const long long id = _wholeFromCents(finalize->number);
	// "finalizar" preserves history and only cuts the future (relative to
	// CURRENT_DATE, resolved at execution time). The type constraint goes in
	// the WHERE because id existence is a runtime check against the DB.
	emitSql("UPDATE operaciones SET estado = 'finalizado'\n");
	emitSql("WHERE id = %lld AND tipo IN ('suscripcion', 'cuotas');\n", id);

	// subscription: closes the recurrence window today. No-op if the id is for installments.
	emitSql("UPDATE suscripciones SET hasta = CURRENT_DATE\n");
	emitSql("WHERE operacion_id = %lld AND (hasta IS NULL OR hasta > CURRENT_DATE);\n", id);

	// installments: cancel only future pending ones and leave the matured
	// ones as history (mark instead of deleting). No-op if the id is a subscription.
	emitSql("UPDATE cuotas SET estado = 'cancelado'\n");
	emitSql("WHERE operacion_id = %lld AND fecha > CURRENT_DATE AND estado = 'pendiente';\n\n", id);
}

static void _generateQuery(const QuerySentence * query) {
	char fromBuffer[ISO_DATE_BUFFER_SIZE];
	char toBuffer[ISO_DATE_BUFFER_SIZE];
	resolvePeriodBounds(query->period, fromBuffer, toBuffer);
	emitSql("SELECT " OPERATION_COLUMNS "\n");
	emitSql("FROM operaciones\n");
	emitSql("WHERE fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	emitSql("ORDER BY fecha, id;\n\n");
}

// Pointer to a function that emits a SELECT (without the trailing ';') for a
// period. It lets _emitReportSavingBlock be generic: the same psql block
// auto-persists PDFs, plain text or HTML depending on the emitter it receives.
typedef void (*ReportSelectEmitter)(const DatePeriod * period);

// Prints the psql block that captures the result of `emitSelect` into a file
// 'reporte_<TIMESTAMP>.<extension>' in the psql client's CWD. The
// meta-commands `\gset`, `\pset`, `\o`, `\echo` belong to the client (not
// standard SQL): other clients will reject them, but the script already
// emitted the "raw" SELECT before with the content, so no information is lost.
static void _emitReportSavingBlock(
	const char * extension,
	const DatePeriod * period,
	ReportSelectEmitter emitSelect
) {
	emitSql("-- Auto-persistencia con psql: si se ejecuta con 'psql -f', el reporte\n");
	emitSql("--    se guarda en el CWD del cliente como\n");
	emitSql("--    'reporte_" DSL_DATE_DISPLAY_MASK "_HHh.MMm.SSs.%s'. Otros clientes rechazaran\n", extension);
	emitSql("--    los meta-comandos pero ya recibieron el contenido en (1).\n");
	emitSql("SELECT 'reporte_' || to_char(now(), '" DSL_DATE_DISPLAY_MASK "\"_\"HH24\"h.\"MI\"m.\"SS\"s\"') || '.%s' AS fname \\gset\n", extension);
	emitSql("\\pset format unaligned\n");
	emitSql("\\pset tuples_only on\n");
	emitSql("\\pset recordsep ''\n");
	emitSql("\\o :fname\n");
	emitSelect(period);
	emitSql(";\n");
	emitSql("\\o\n");
	emitSql("\\pset format aligned\n");
	emitSql("\\pset tuples_only off\n");
	emitSql("\\pset recordsep '\\n'\n");
	emitSql("\\echo 'Reporte guardado en' :fname\n\n");
}

static void _generateReport(const ReportSentence * report) {
	const DatePeriod * period = report->period;
	const char * extension = NULL;
	ReportSelectEmitter emitSelect = NULL;

	switch (report->format->kind) {
		case REPORT_FORMAT_HTML:
			emitSql("-- Reporte (formato HTML: documento HTML completo armado en SQL).\n");
			extension = "html";
			emitSelect = emitHtmlReportSelect;
			break;
		case REPORT_FORMAT_PLAIN_TEXT:
			emitSql("-- Reporte (formato texto plano: tabla ASCII alineada).\n");
			extension = "txt";
			emitSelect = emitTextReportSelect;
			break;
		case REPORT_FORMAT_PDF:
			emitSql("-- Reporte (formato PDF: documento PDF 1.4 armado en SQL).\n");
			extension = "pdf";
			emitSelect = emitPdfReportSelect;
			break;
		default:
			return;
	}

	// 1) Raw SELECT: any client receives the report contents as a single
	//    row/column. The PDF comes back as text; the HTML as a full document;
	//    the plain text as one row per operation.
	emitSql("-- 1) SELECT puro: devuelve el contenido del reporte (cualquier cliente).\n");
	emitSelect(period);
	emitSql(";\n\n");

	// 2) Block of psql meta-commands that copies the same SELECT to a file
	//    with timestamp in the client's CWD.
	_emitReportSavingBlock(extension, period, emitSelect);
}

static void _generateSentence(const Sentence * sentence, const char * currency) {
	switch (sentence->kind) {
		case SENTENCE_EXPENSE:
			_generateExpense(sentence->expenseSentence, currency);
			break;
		case SENTENCE_INCOME:
			_generateIncome(sentence->incomeSentence, currency);
			break;
		case SENTENCE_SUBSCRIPTION:
			_generateSubscription(sentence->subscriptionSentence, currency);
			break;
		case SENTENCE_QUERY:
			_generateQuery(sentence->querySentence);
			break;
		case SENTENCE_EDIT:
			_generateEdit(sentence->editSentence);
			break;
		case SENTENCE_DELETE:
			_generateDelete(sentence->deleteSentence);
			break;
		case SENTENCE_REPORT:
			_generateReport(sentence->reportSentence);
			break;
		case SENTENCE_FINALIZE:
			_generateFinalize(sentence->finalizeSentence);
			break;
		default:
			logError(_logger, "The specified sentence kind is unknown: %d", sentence->kind);
			break;
	}
}

/* PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
	logDebugging(_logger, "Generating PostgreSQL script...");
	Program * program = compilerState->abstractSyntaxTree;

	// Active currency, tracked "live" during generation. The symbol table
	// only stores the LAST currency declared in the program, so it is
	// useless for intermediate sentences. The default ('ARS') applies to
	// any operation prior to the first 'divisa <ID>'.
	const char * activeCurrency = "ARS";

	_generatePrologue();

	for (Sentences * sentences = program->sentences; sentences != NULL; sentences = sentences->next) {
		const Sentence * sentence = sentences->sentence;
		if (sentence->kind == SENTENCE_CURRENCY) {
			activeCurrency = sentence->currencySentence->id;
			emitSql("-- divisa activa: %s\n", activeCurrency);
		}
		else {
			_generateSentence(sentence, activeCurrency);
		}
	}

	flushSql();
	logDebugging(_logger, "Generation is done.");
}
