#include "Generator.h"
#include "../domain-specific/HtmlReport.h"
#include "../domain-specific/PdfReport.h"
#include "../domain-specific/ReportModel.h"
#include "../domain-specific/TextReport.h"
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

// columnas de una operacion, compartidas por queries (consultar) y por la
// proyeccion plana que comparten todas las sentencias INSERT/UPDATE.
#define OPERATION_COLUMNS "id, tipo, monto, divisa, categoria, fecha, descripcion, estado"

/* PRIVATE FUNCTIONS */

// Los montos viajan en centavos como long long; al SQL salen con dos decimales
// para encajar en NUMERIC(15,2). Los ids/cuotas tambien viajan en centavos pero
// el analisis semantico ya garantizo que su parte fraccionaria es 0, asi que
// "centavos / 100" reconstruye el entero original sin perdida.
static void _emitAmount(long long centavos) {
	const long long whole = centavos / 100;
	const long long cents = (centavos % 100 + 100) % 100;
	emitSql("%lld.%02lld", whole, cents);
}

static long long _wholeFromCentavos(long long centavos) {
	return centavos / 100;
}

// escapa un string para meterlo en un literal SQL entre comillas simples
// (cada ' se duplica). devuelve heap que libera el caller; no agrega comillas.
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

// normaliza la categoria (reusa la logica de la tabla de simbolos) y la escapa
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

// fecha base de una operacion: la propia si la tiene, si no hoy. escribe el
// ISO en "buffer" (>= 11 bytes) y devuelve el YYYYMMDD para seguir operando.
static DateValue _baseDate(const Date * date, char * buffer) {
	const DateValue value = (date != NULL) ? _resolveDate(date) : today();
	formatDateValueIso(value, buffer);
	return value;
}

/* DDL */

// esquema relacional idempotente. operaciones es la tabla ancla (su id serial
// es lo que editar/eliminar/finalizar tocan en runtime); cuotas y suscripciones
// cuelgan de ella por FK.
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

// categoria opcional como valor SQL: literal normalizado entre comillas o NULL
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
	char base[11];
	const Date * date = (expense->optionalDate != NULL) ? expense->optionalDate->date : NULL;
	_baseDate(date, base);

	if (expense->optionalInstallments == NULL) {
		// gasto simple: una sola fila
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

	// gasto en cuotas: la operacion padre + N obligaciones futuras, una por mes
	// desde la fecha base, todas con el id del padre (RETURNING).
	const long long count = _wholeFromCentavos(expense->optionalInstallments->count);
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
	char base[11];
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
	char fromBuffer[11];
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
		char untilBuffer[11];
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
				char buffer[11];
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
	emitSql(" WHERE id = %lld;\n\n", _wholeFromCentavos(edit->number));
}

static void _generateDelete(const DeleteSentence * del) {
	emitSql("DELETE FROM operaciones WHERE id = %lld;\n\n", _wholeFromCentavos(del->number));
}

static void _generateFinalize(const FinalizeSentence * finalize) {
	const long long id = _wholeFromCentavos(finalize->number);
	// finalizar conserva el historial y solo corta lo futuro (relativo a
	// CURRENT_DATE, que se resuelve al ejecutar). La restriccion de tipo va en
	// el WHERE porque la existencia del id es un chequeo de runtime contra la DB.
	emitSql("UPDATE operaciones SET estado = 'finalizado'\n");
	emitSql("WHERE id = %lld AND tipo IN ('suscripcion', 'cuotas');\n", id);

	// suscripcion: cierra la ventana de recurrencia hoy. inocuo si el id es cuotas.
	emitSql("UPDATE suscripciones SET hasta = CURRENT_DATE\n");
	emitSql("WHERE operacion_id = %lld AND (hasta IS NULL OR hasta > CURRENT_DATE);\n", id);

	// cuotas: cancela solo las futuras pendientes y deja las vencidas como
	// historial (marca en vez de borrar). inocuo si el id es una suscripcion.
	emitSql("UPDATE cuotas SET estado = 'cancelado'\n");
	emitSql("WHERE operacion_id = %lld AND fecha > CURRENT_DATE AND estado = 'pendiente';\n\n", id);
}

static void _generateQuery(const QuerySentence * query) {
	char fromBuffer[11];
	char toBuffer[11];
	resolvePeriodBounds(query->period, fromBuffer, toBuffer);
	emitSql("SELECT " OPERATION_COLUMNS "\n");
	emitSql("FROM operaciones\n");
	emitSql("WHERE fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	emitSql("ORDER BY fecha, id;\n\n");
}

// Puntero a una funcion que emite un SELECT (sin ';' final) para un periodo.
// Permite que _emitReportSavingBlock sea generico: el mismo bloque psql
// auto-persiste PDFs, texto plano o HTML segun el emisor que recibe.
typedef void (*ReportSelectEmitter)(const DatePeriod * period);

// Imprime el bloque psql que captura el resultado de `emitSelect` a un archivo
// 'reporte_<TIMESTAMP>.<extension>' en el CWD del cliente psql. Los meta-
// comandos `\gset`, `\pset`, `\o`, `\echo` son del cliente (no SQL estandar):
// otros clientes los van a rechazar, pero el script ya emitio antes el SELECT
// "puro" con el contenido, asi que la informacion no se pierde.
static void _emitReportSavingBlock(
	const char * extension,
	const DatePeriod * period,
	ReportSelectEmitter emitSelect
) {
	emitSql("-- Auto-persistencia con psql: si se ejecuta con 'psql -f', el reporte\n");
	emitSql("--    se guarda en el CWD del cliente como\n");
	emitSql("--    'reporte_DD-MM-YYYY_HHh.MMm.SSs.%s'. Otros clientes rechazaran\n", extension);
	emitSql("--    los meta-comandos pero ya recibieron el contenido en (1).\n");
	emitSql("SELECT 'reporte_' || to_char(now(), 'DD-MM-YYYY\"_\"HH24\"h.\"MI\"m.\"SS\"s\"') || '.%s' AS fname \\gset\n", extension);
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

	// 1) SELECT puro: cualquier cliente recibe el contenido del reporte como
	//    una fila/columna. El PDF se devuelve como text; el HTML como un
	//    documento completo; el texto plano como una fila por operacion.
	emitSql("-- 1) SELECT puro: devuelve el contenido del reporte (cualquier cliente).\n");
	emitSelect(period);
	emitSql(";\n\n");

	// 2) Bloque de meta-comandos psql que copia el mismo SELECT a un archivo
	//    con timestamp en el CWD del cliente.
	_emitReportSavingBlock(extension, period, emitSelect);
}

static void _generateSentence(const Sentence * sentence, const char * currency) {
	switch (sentence->kind) {
		case SENTENCE_CURRENCY:
			// la divisa activa sale de la tabla de simbolos; aca solo dejamos rastro
			emitSql("-- divisa activa: %s\n", sentence->currencySentence->id);
			break;
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
	const char * currency = (compilerState->symbolTable != NULL)
		? compilerState->symbolTable->activeCurrency
		: "ARS";
	_generatePrologue();
	for (Sentences * sentences = program->sentences; sentences != NULL; sentences = sentences->next) {
		_generateSentence(sentences->sentence, currency);
	}
	logDebugging(_logger, "Generation is done.");
}
