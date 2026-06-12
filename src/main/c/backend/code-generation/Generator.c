#include "Generator.h"
#include "../../support/language/DateUtils.h"
#include "../../support/language/String.h"
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

// columnas de una operacion, compartidas por queries y reportes (id primero)
#define OPERATION_COLUMNS "id, tipo, monto, divisa, categoria, fecha, descripcion, estado"

/* PRIVATE FUNCTIONS */

// imprime una linea al script SQL (stdout). el fflush deja la salida visible
// aunque una fase posterior aborte.
static void _out(const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	vfprintf(stdout, format, arguments);
	va_end(arguments);
	fflush(stdout);
}

// Los montos viajan en centavos como long long; al SQL salen con dos decimales
// para encajar en NUMERIC(15,2). Los ids/cuotas tambien viajan en centavos pero
// el analisis semantico ya garantizo que su parte fraccionaria es 0, asi que
// "centavos / 100" reconstruye el entero original sin perdida.
static void _emitAmount(long long centavos) {
	const long long whole = centavos / 100;
	const long long cents = (centavos % 100 + 100) % 100;
	_out("%lld.%02lld", whole, cents);
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
	_out("-- =====================================================================\n");
	_out("-- Script SQL generado por el compilador del DSL de finanzas (PostgreSQL)\n");
	_out("-- =====================================================================\n\n");
	_out("-- Esquema relacional (idempotente).\n");
	_out(
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
	_out(
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
	_out(
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
	_out("-- Sentencias del programa.\n");
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
		_out("NULL");
		return;
	}
	char * literal = _categoryLiteral(category->id);
	_out("'%s'", literal);
	free(literal);
}

static void _emitDescriptionValue(const OptionalDescription * description) {
	if (description == NULL) {
		_out("NULL");
		return;
	}
	char * escaped = _sqlEscape(description->text);
	_out("'%s'", escaped);
	free(escaped);
}

static void _generateExpense(const ExpenseSentence * expense, const char * currency) {
	char base[11];
	const Date * date = (expense->optionalDate != NULL) ? expense->optionalDate->date : NULL;
	_baseDate(date, base);

	if (expense->optionalInstallments == NULL) {
		// gasto simple: una sola fila
		_out("INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
		_out("VALUES ('gasto', ");
		_emitAmount(expense->number);
		_out(", '%s', ", currency);
		_emitCategoryValue(expense->optionalCategory);
		_out(", DATE '%s', ", base);
		_emitDescriptionValue(expense->optionalDescription);
		_out(");\n\n");
		return;
	}

	// gasto en cuotas: la operacion padre + N obligaciones futuras, una por mes
	// desde la fecha base, todas con el id del padre (RETURNING).
	const long long count = _wholeFromCentavos(expense->optionalInstallments->count);
	_out("WITH nueva AS (\n");
	_out("    INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	_out("    VALUES ('cuotas', ");
	_emitAmount(expense->number);
	_out(", '%s', ", currency);
	_emitCategoryValue(expense->optionalCategory);
	_out(", DATE '%s', ", base);
	_emitDescriptionValue(expense->optionalDescription);
	_out(")\n");
	_out("    RETURNING id, fecha\n");
	_out(")\n");
	_out("INSERT INTO cuotas (operacion_id, numero_cuota, total_cuotas, monto, divisa, fecha)\n");
	_out("SELECT nueva.id, gs.n, %lld, ROUND(", count);
	_emitAmount(expense->number);
	_out("::numeric / %lld, 2), '%s',\n", count, currency);
	_out("       (nueva.fecha + ((gs.n - 1) * INTERVAL '1 month'))::date\n");
	_out("FROM nueva, generate_series(1, %lld) AS gs(n);\n\n", count);
}

static void _generateIncome(const IncomeSentence * income, const char * currency) {
	char base[11];
	const Date * date = (income->optionalDate != NULL) ? income->optionalDate->date : NULL;
	_baseDate(date, base);

	_out("INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	_out("VALUES ('ingreso', ");
	_emitAmount(income->number);
	_out(", '%s', ", currency);
	_emitCategoryValue(income->optionalCategory);
	_out(", DATE '%s', ", base);
	_emitDescriptionValue(income->optionalDescription);
	_out(");\n\n");
}

static void _generateSubscription(const SubscriptionSentence * subscription, const char * currency) {
	char fromBuffer[11];
	const Date * fromDate = (subscription->optionalFrom != NULL) ? subscription->optionalFrom->date : NULL;
	_baseDate(fromDate, fromBuffer);

	_out("WITH nueva AS (\n");
	_out("    INSERT INTO operaciones (tipo, monto, divisa, categoria, fecha, descripcion)\n");
	_out("    VALUES ('suscripcion', ");
	_emitAmount(subscription->number);
	_out(", '%s', ", currency);
	_emitCategoryValue(subscription->optionalCategory);
	_out(", DATE '%s', ", fromBuffer);
	_emitDescriptionValue(subscription->optionalDescription);
	_out(")\n");
	_out("    RETURNING id\n");
	_out(")\n");
	_out("INSERT INTO suscripciones (operacion_id, monto, divisa, categoria, frecuencia, desde, hasta, descripcion)\n");
	_out("SELECT nueva.id, ");
	_emitAmount(subscription->number);
	_out(", '%s', ", currency);
	_emitCategoryValue(subscription->optionalCategory);
	_out(", '%s', DATE '%s', ", _frequencyName(subscription->frequency), fromBuffer);
	if (subscription->optionalUntil != NULL) {
		char untilBuffer[11];
		formatDateValueIso(_resolveDate(subscription->optionalUntil->date), untilBuffer);
		_out("DATE '%s', ", untilBuffer);
	}
	else {
		_out("NULL, ");
	}
	_emitDescriptionValue(subscription->optionalDescription);
	_out("\nFROM nueva;\n\n");
}

static void _generateEdit(const EditSentence * edit) {
	_out("UPDATE operaciones SET ");
	bool first = true;
	for (EditFieldList * node = edit->fields; node != NULL; node = node->next) {
		EditField * field = node->field;
		if (!first) {
			_out(", ");
		}
		first = false;
		switch (field->kind) {
			case EDIT_FIELD_AMOUNT:
				_out("monto = ");
				_emitAmount(field->amount);
				break;
			case EDIT_FIELD_CATEGORY: {
				char * literal = _categoryLiteral(field->categoryId);
				_out("categoria = '%s'", literal);
				free(literal);
				break;
			}
			case EDIT_FIELD_DATE: {
				char buffer[11];
				formatDateValueIso(_resolveDate(field->date), buffer);
				_out("fecha = DATE '%s'", buffer);
				break;
			}
			case EDIT_FIELD_DESCRIPTION: {
				char * escaped = _sqlEscape(field->description);
				_out("descripcion = '%s'", escaped);
				free(escaped);
				break;
			}
			default:
				break;
		}
	}
	_out(" WHERE id = %lld;\n\n", _wholeFromCentavos(edit->number));
}

static void _generateDelete(const DeleteSentence * del) {
	_out("DELETE FROM operaciones WHERE id = %lld;\n\n", _wholeFromCentavos(del->number));
}

static void _generateFinalize(const FinalizeSentence * finalize) {
	const long long id = _wholeFromCentavos(finalize->number);
	// finalizar conserva el historial y solo corta lo futuro (relativo a
	// CURRENT_DATE, que se resuelve al ejecutar). La restriccion de tipo va en
	// el WHERE porque la existencia del id es un chequeo de runtime contra la DB.
	_out("UPDATE operaciones SET estado = 'finalizado'\n");
	_out("WHERE id = %lld AND tipo IN ('suscripcion', 'cuotas');\n", id);

	// suscripcion: cierra la ventana de recurrencia hoy. inocuo si el id es cuotas.
	_out("UPDATE suscripciones SET hasta = CURRENT_DATE\n");
	_out("WHERE operacion_id = %lld AND (hasta IS NULL OR hasta > CURRENT_DATE);\n", id);

	// cuotas: cancela solo las futuras pendientes y deja las vencidas como
	// historial (marca en vez de borrar). inocuo si el id es una suscripcion.
	_out("UPDATE cuotas SET estado = 'cancelado'\n");
	_out("WHERE operacion_id = %lld AND fecha > CURRENT_DATE AND estado = 'pendiente';\n\n", id);
}

// filtro de periodo como "fecha BETWEEN ... AND ...". un rango usa sus extremos
// (ya validados); una frecuencia es una ventana que cierra hoy y va un periodo
// para atras.
static void _emitPeriodFilter(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	if (period->kind == DATE_PERIOD_RANGE) {
		formatDateValueIso(_resolveDate(period->fromDate), fromBuffer);
		formatDateValueIso(_resolveDate(period->toDate), toBuffer);
	}
	else {
		const DateValue close = today();
		DateValue start = close;
		switch (period->frequency->kind) {
			case FREQUENCY_MONTHLY: start = addMonths(close, -1); break;
			case FREQUENCY_WEEKLY: start = addDays(close, -7); break;
			case FREQUENCY_YEARLY: start = addMonths(close, -12); break;
			default: start = addMonths(close, -1); break;
		}
		formatDateValueIso(start, fromBuffer);
		formatDateValueIso(close, toBuffer);
	}
	_out("WHERE fecha BETWEEN DATE '%s' AND DATE '%s'", fromBuffer, toBuffer);
}

static void _generatePlainSelect(const DatePeriod * period) {
	_out("SELECT " OPERATION_COLUMNS "\n");
	_out("FROM operaciones\n");
	_emitPeriodFilter(period);
	_out("\nORDER BY fecha, id;\n\n");
}

static void _generateQuery(const QuerySentence * query) {
	_generatePlainSelect(query->period);
}

// arma el resultado como una tabla HTML (header + una fila por operacion),
// agregando del lado del server con string_agg.
static void _generateHtmlReport(const DatePeriod * period) {
	_out("SELECT '<table>'\n");
	_out("    || '<thead><tr>'\n");
	_out("    || '<th>id</th><th>tipo</th><th>monto</th><th>divisa</th>'\n");
	_out("    || '<th>categoria</th><th>fecha</th><th>descripcion</th><th>estado</th>'\n");
	_out("    || '</tr></thead><tbody>'\n");
	_out("    || COALESCE(string_agg(\n");
	_out("        '<tr>'\n");
	_out("        || '<td>' || id::text || '</td>'\n");
	_out("        || '<td>' || tipo || '</td>'\n");
	_out("        || '<td>' || monto::text || '</td>'\n");
	_out("        || '<td>' || divisa || '</td>'\n");
	_out("        || '<td>' || COALESCE(categoria, '') || '</td>'\n");
	_out("        || '<td>' || fecha::text || '</td>'\n");
	_out("        || '<td>' || COALESCE(descripcion, '') || '</td>'\n");
	_out("        || '<td>' || estado || '</td>'\n");
	_out("        || '</tr>', '' ORDER BY fecha, id), '')\n");
	_out("    || '</tbody></table>' AS reporte_html\n");
	_out("FROM operaciones\n");
	_emitPeriodFilter(period);
	_out(";\n\n");
}

static void _generateReport(const ReportSentence * report) {
	switch (report->format->kind) {
		case REPORT_FORMAT_HTML:
			_out("-- Reporte (formato HTML).\n");
			_generateHtmlReport(report->period);
			break;
		case REPORT_FORMAT_PLAIN_TEXT:
			_out("-- Reporte (formato texto plano).\n");
			_generatePlainSelect(report->period);
			break;
		case REPORT_FORMAT_PDF:
			// el render binario del PDF queda para runtime; emitimos la query
			// con los datos para que el runtime los formatee
			_out("-- Reporte (formato PDF: render runtime futuro).\n");
			_generatePlainSelect(report->period);
			break;
		default:
			break;
	}
}

static void _generateSentence(const Sentence * sentence, const char * currency) {
	switch (sentence->kind) {
		case SENTENCE_CURRENCY:
			// la divisa activa sale de la tabla de simbolos; aca solo dejamos rastro
			_out("-- divisa activa: %s\n", sentence->currencySentence->id);
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
