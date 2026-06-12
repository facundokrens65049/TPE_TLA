#include "Generator.h"
#include "../../support/language/DateUtils.h"
#include "../../support/language/String.h"
#include <stdbool.h>
#include <stdlib.h>

#define COL_ID_WIDTH 5
#define COL_TIPO_WIDTH 14
#define COL_MONTO_WIDTH 13
#define COL_DIVISA_WIDTH 7
#define COL_CATEGORIA_WIDTH 18
#define COL_FECHA_WIDTH 10
#define COL_DETALLE_WIDTH 8
#define COL_DESCRIPCION_WIDTH 50

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

// extremos del periodo en ISO. un rango usa los extremos (ya validados); una
// frecuencia es una ventana que cierra hoy y va un periodo para atras.
static void _resolvePeriodBounds(const DatePeriod * period, char * fromBuffer, char * toBuffer) {
	if (period->kind == DATE_PERIOD_RANGE) {
		formatDateValueIso(_resolveDate(period->fromDate), fromBuffer);
		formatDateValueIso(_resolveDate(period->toDate), toBuffer);
		return;
	}
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

// filtro de periodo como "fecha BETWEEN ... AND ...".
static void _emitPeriodFilter(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	_resolvePeriodBounds(period, fromBuffer, toBuffer);
	_out("WHERE fecha BETWEEN DATE '%s' AND DATE '%s'", fromBuffer, toBuffer);
}

// Emite un SELECT sin ';\n' final. Devuelve una fila por operacion del
// periodo; usado por `consultar` y como base del reporte texto plano.
static void _emitPlainSelect(const DatePeriod * period) {
	_out("SELECT " OPERATION_COLUMNS "\n");
	_out("FROM operaciones\n");
	_emitPeriodFilter(period);
	_out("\nORDER BY fecha, id");
}

static void _generateQuery(const QuerySentence * query) {
	_emitPlainSelect(query->period);
	_out(";\n\n");
}

// Emite las CTEs comunes a los reportes de texto plano y PDF.
static void _emitReportRowsCTE(const char * fromBuffer, const char * toBuffer, bool pdfEscape) {
	_out("filas_base AS (\n");
	_out("    -- Gastos e ingresos puntuales.\n");
	_out("    SELECT id, tipo, monto, divisa, categoria, fecha, descripcion,\n");
	_out("           CAST(NULL AS text) AS detalle\n");
	_out("    FROM operaciones\n");
	_out("    WHERE tipo IN ('gasto', 'ingreso')\n");
	_out("      AND fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	_out("    UNION ALL\n");
	_out("    -- Cuotas individuales: una fila por cuota con su fecha real, monto\n");
	_out("    -- prorrateado y detalle 'k/N'. La fila padre de cuotas no aparece.\n");
	_out("    SELECT op.id, 'cuota'::text, c.monto, c.divisa, op.categoria,\n");
	_out("           c.fecha, op.descripcion,\n");
	_out("           c.numero_cuota::text || '/' || c.total_cuotas::text\n");
	_out("    FROM cuotas c JOIN operaciones op ON op.id = c.operacion_id\n");
	_out("    WHERE c.fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	_out("    UNION ALL\n");
	_out("    -- Suscripciones: la fila padre + frecuencia como detalle.\n");
	_out("    SELECT op.id, op.tipo, op.monto, op.divisa, op.categoria,\n");
	_out("           op.fecha, op.descripcion, s.frecuencia\n");
	_out("    FROM operaciones op JOIN suscripciones s ON s.operacion_id = op.id\n");
	_out("    WHERE op.tipo = 'suscripcion'\n");
	_out("      AND op.fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	_out("),\n");
	_out("filas_wrap AS (\n");
	_out("    -- Lineas necesarias para envolver categoria/descripcion sin cortar.\n");
	_out("    SELECT *,\n");
	_out("           GREATEST(\n");
	_out("               CEIL(GREATEST(length(COALESCE(categoria, '')), 1)::numeric / %d)::int,\n", COL_CATEGORIA_WIDTH);
	_out("               CEIL(GREATEST(length(COALESCE(descripcion, '')), 1)::numeric / %d)::int,\n", COL_DESCRIPCION_WIDTH);
	_out("               1\n");
	_out("           ) AS line_count\n");
	_out("    FROM filas_base\n");
	_out("),\n");
	_out("filas_expandidas AS (\n");
	_out("    -- Una fila por linea visible (j = 0..line_count-1).\n");
	_out("    SELECT f.*, gs.n - 1 AS j\n");
	_out("    FROM filas_wrap f, generate_series(1, f.line_count) AS gs(n)\n");
	_out("),\n");
	_out("filas_lineas AS (\n");
	_out("    -- Composicion final: solo j=0 muestra columnas estaticas; categoria\n");
	_out("    -- y descripcion van wrap-eadas en ventanas de su ancho.\n");
	_out("    SELECT\n");
	_out("        CASE WHEN j = 0 THEN rpad(id::text, %d)         ELSE rpad('', %d) END || ' ' ||\n", COL_ID_WIDTH, COL_ID_WIDTH);
	_out("        CASE WHEN j = 0 THEN rpad(left(tipo, %d), %d)    ELSE rpad('', %d) END || ' ' ||\n", COL_TIPO_WIDTH, COL_TIPO_WIDTH, COL_TIPO_WIDTH);
	_out("        CASE WHEN j = 0 THEN lpad(monto::text, %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_MONTO_WIDTH, COL_MONTO_WIDTH);
	_out("        CASE WHEN j = 0 THEN rpad(left(divisa, %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_DIVISA_WIDTH, COL_DIVISA_WIDTH, COL_DIVISA_WIDTH);
	_out("        rpad(COALESCE(substring(COALESCE(categoria, '') FROM j * %d + 1 FOR %d), ''), %d) || ' ' ||\n", COL_CATEGORIA_WIDTH, COL_CATEGORIA_WIDTH, COL_CATEGORIA_WIDTH);
	_out("        CASE WHEN j = 0 THEN rpad(fecha::text, %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_FECHA_WIDTH, COL_FECHA_WIDTH);
	_out("        CASE WHEN j = 0 THEN rpad(left(COALESCE(detalle, ''), %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_DETALLE_WIDTH, COL_DETALLE_WIDTH, COL_DETALLE_WIDTH);
	_out("        rpad(COALESCE(substring(COALESCE(descripcion, '') FROM j * %d + 1 FOR %d), ''), %d) AS linea_raw,\n", COL_DESCRIPCION_WIDTH, COL_DESCRIPCION_WIDTH, COL_DESCRIPCION_WIDTH);
	_out("        fecha, id, j\n");
	_out("    FROM filas_expandidas\n");
	_out(")");
	if (pdfEscape) {
		_out(",\n");
		_out("filas_alineadas AS (\n");
		_out("    -- Escape de los caracteres reservados de los string literals PDF.\n");
		_out("    SELECT replace(replace(replace(replace(replace(replace(\n");
		_out("        linea_raw,\n");
		_out("        chr(92), chr(92) || chr(92)),\n");
		_out("        '(',     chr(92) || '('),\n");
		_out("        ')',     chr(92) || ')'),\n");
		_out("        chr(10), chr(92) || 'n'),\n");
		_out("        chr(13), chr(92) || 'r'),\n");
		_out("        chr(9),  chr(92) || 't') AS texto,\n");
		_out("        ROW_NUMBER() OVER (ORDER BY fecha, id, j) AS rn\n");
		_out("    FROM filas_lineas\n");
		_out(")");
	}
	_out(",\n");
	_out("balance AS (\n");
	_out("    -- Balance neto del periodo agrupado por divisa: ingresos como aporte\n");
	_out("    -- positivo, todo lo demas (gastos, cuotas individuales, suscripciones)\n");
	_out("    -- como egreso. El balance es la suma EXACTA de los montos visibles en\n");
	_out("    -- la tabla (con sus signos), asi que cuadra con la columna 'monto'.\n");
	_out("    SELECT divisa,\n");
	_out("           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE 0 END) AS ingresos,\n");
	_out("           SUM(CASE WHEN tipo <> 'ingreso' THEN monto ELSE 0 END) AS egresos,\n");
	_out("           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE -monto END) AS neto\n");
	_out("    FROM filas_base\n");
	_out("    GROUP BY divisa\n");
	_out(")");
}

// Emite un SELECT sin ';\n' final que devuelve UNA SOLA fila con el reporte
// formateado como tabla de ancho fijo (cabecera + separador + filas con wrap
// multi-linea). Mismas columnas y anchos que el PDF.
static void _emitTextReportSelect(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	_resolvePeriodBounds(period, fromBuffer, toBuffer);

	_out("WITH\n");
	_emitReportRowsCTE(fromBuffer, toBuffer, false);
	_out(",\n");
	_out("cabecera AS (\n");
	_out("    SELECT\n");
	_out("        rpad('id', %d) || ' ' || rpad('tipo', %d) || ' ' || lpad('monto', %d) || ' ' ||\n", COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	_out("        rpad('divisa', %d) || ' ' || rpad('categoria', %d) || ' ' || rpad('fecha', %d) || ' ' ||\n", COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	_out("        rpad('detalle', %d) || ' ' || 'descripcion' AS header_line,\n", COL_DETALLE_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n", COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n", COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) AS separator_line\n", COL_DETALLE_WIDTH, COL_DESCRIPCION_WIDTH);
	_out(")\n");
	_out("SELECT 'Reporte de operaciones' || chr(10) ||\n");
	_out("       'Periodo: %s a %s' || chr(10) || chr(10) ||\n", fromBuffer, toBuffer);
	_out("       (SELECT header_line FROM cabecera) || chr(10) ||\n");
	_out("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	_out("       COALESCE(\n");
	_out("           (SELECT string_agg(linea_raw, chr(10) ORDER BY fecha, id, j) FROM filas_lineas),\n");
	_out("           '(sin operaciones en el periodo)'\n");
	_out("       ) || chr(10) || chr(10) ||\n");
	_out("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	_out("       'Balance del periodo' || chr(10) ||\n");
	_out("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	_out("       COALESCE(\n");
	_out("           (SELECT string_agg(\n");
	_out("               rpad(divisa, %d) || ' ingresos: ' || lpad(to_char(ingresos, 'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	_out("               rpad(divisa, %d) || ' egresos:  ' || lpad(to_char(egresos,  'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	_out("               rpad(divisa, %d) || ' balance:  ' || lpad(to_char(neto,     'FM9999999990.00'), 15),\n", COL_DIVISA_WIDTH);
	_out("               chr(10) ORDER BY divisa\n");
	_out("           ) FROM balance),\n");
	_out("           '(sin movimientos en el periodo)'\n");
	_out("       ) || chr(10) AS reporte_texto");
}

// Emite un SELECT sin ';\n' final que devuelve un documento HTML5 completo
// (DOCTYPE + <head> con CSS inline + <body> con tabla) listo para abrir en el
// navegador. Usa el mismo modelo de filas que texto/PDF (UNION ALL para
// explotar cuotas individuales y mostrar 'detalle' en vez de 'estado'), pero
// sin wrap manual: <td> ya envuelve el contenido al ancho de la celda.
static void _emitHtmlSelect(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	_resolvePeriodBounds(period, fromBuffer, toBuffer);

	_out("WITH\n");
	_emitReportRowsCTE(fromBuffer, toBuffer, false);
	_out("\n");
	_out("SELECT '<!DOCTYPE html>'\n");
	_out("    || '<html lang=\"es\"><head><meta charset=\"utf-8\">'\n");
	_out("    || '<title>Reporte de operaciones</title>'\n");
	_out("    || '<style>body{font-family:system-ui,sans-serif;margin:24px;color:#222}'\n");
	_out("    || 'h1{font-size:18px;margin:0 0 4px}h2{font-size:15px;margin:24px 0 8px}'\n");
	_out("    || 'p{margin:0 0 16px;color:#666}'\n");
	_out("    || 'table{border-collapse:collapse;width:100%%;font-size:14px}'\n");
	_out("    || 'th,td{border:1px solid #ddd;padding:6px 10px;text-align:left;vertical-align:top}'\n");
	_out("    || 'th{background:#f3f4f6}tr:nth-child(even) td{background:#fafafa}'\n");
	_out("    || 'td.num{text-align:right;font-variant-numeric:tabular-nums}'\n");
	_out("    || 'td.det{white-space:nowrap;color:#555}'\n");
	_out("    || 'table.balance{width:auto;min-width:380px}'\n");
	_out("    || 'table.balance tr.neto td{font-weight:bold;background:#eef6ff}'\n");
	_out("    || 'td.neg{color:#b00020}td.pos{color:#0a7a2a}'\n");
	_out("    || '</style></head><body>'\n");
	_out("    || '<h1>Reporte de operaciones</h1>'\n");
	_out("    || '<p>Periodo: %s a %s</p>'\n", fromBuffer, toBuffer);
	_out("    || '<table>'\n");
	_out("    || '<thead><tr>'\n");
	_out("    || '<th>id</th><th>tipo</th><th>monto</th><th>divisa</th>'\n");
	_out("    || '<th>categoria</th><th>fecha</th><th>detalle</th><th>descripcion</th>'\n");
	_out("    || '</tr></thead><tbody>'\n");
	_out("    || COALESCE((SELECT string_agg(\n");
	_out("        '<tr>'\n");
	_out("        || '<td class=\"num\">' || id::text || '</td>'\n");
	_out("        || '<td>' || tipo || '</td>'\n");
	_out("        || '<td class=\"num\">' || monto::text || '</td>'\n");
	_out("        || '<td>' || divisa || '</td>'\n");
	_out("        || '<td>' || COALESCE(categoria, '') || '</td>'\n");
	_out("        || '<td>' || fecha::text || '</td>'\n");
	_out("        || '<td class=\"det\">' || COALESCE(detalle, '') || '</td>'\n");
	_out("        || '<td>' || COALESCE(descripcion, '') || '</td>'\n");
	_out("        || '</tr>', '' ORDER BY fecha, id) FROM filas_base),\n");
	_out("        '<tr><td colspan=\"8\" style=\"text-align:center;color:#888\">Sin operaciones en el periodo.</td></tr>'\n");
	_out("    )\n");
	_out("    || '</tbody></table>'\n");
	_out("    || '<h2>Balance del periodo</h2>'\n");
	_out("    || COALESCE((SELECT '<table class=\"balance\"><thead><tr>'\n");
	_out("        || '<th>divisa</th><th>ingresos</th><th>egresos</th><th>balance neto</th>'\n");
	_out("        || '</tr></thead><tbody>'\n");
	_out("        || string_agg(\n");
	_out("            '<tr class=\"neto\">'\n");
	_out("            || '<td>' || divisa || '</td>'\n");
	_out("            || '<td class=\"num pos\">' || to_char(ingresos, 'FM9999999990.00') || '</td>'\n");
	_out("            || '<td class=\"num neg\">' || to_char(egresos,  'FM9999999990.00') || '</td>'\n");
	_out("            || '<td class=\"num ' || CASE WHEN neto >= 0 THEN 'pos' ELSE 'neg' END || '\">'\n");
	_out("            || to_char(neto, 'FM9999999990.00') || '</td>'\n");
	_out("            || '</tr>', '' ORDER BY divisa)\n");
	_out("        || '</tbody></table>' FROM balance),\n");
	_out("        '<p style=\"color:#888\">Sin movimientos en el periodo.</p>'\n");
	_out("    )\n");
	_out("    || '</body></html>' || chr(10) AS reporte_html");
}

// Arma un PDF 1.4 paginado (fuente Courier, hasta N filas por pagina) en SQL
// puro. Devuelve una sola fila con el documento completo como text.
#define PDF_LINES_PER_PAGE 30

// Emite el WITH ... SELECT que arma el documento PDF. Se llama dos veces desde
// _generateReport: una "pura" (compatible con cualquier cliente) y otra dentro
// del bloque psql que captura el output a archivo. No imprime el ';\n' final
// (lo agrega el caller, asi controla el separador de sentencias).
static void _emitPdfSelect(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	_resolvePeriodBounds(period, fromBuffer, toBuffer);
	_out("WITH\n");
	_out("-- Cabecera de la tabla (texto fijo). Las columnas se alinean con rpad/lpad\n");
	_out("-- porque Courier es monoespaciada; el separador es una linea de guiones.\n");
	_out("cabecera AS (\n");
	_out("    SELECT\n");
	_out("        rpad('id', %d) || ' ' || rpad('tipo', %d) || ' ' || lpad('monto', %d) || ' ' ||\n", COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	_out("        rpad('divisa', %d) || ' ' || rpad('categoria', %d) || ' ' || rpad('fecha', %d) || ' ' ||\n", COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	_out("        rpad('detalle', %d) || ' ' || 'descripcion' AS header_line,\n", COL_DETALLE_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n", COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n", COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	_out("        repeat('-', %d) || ' ' || repeat('-', %d) AS separator_line\n", COL_DETALLE_WIDTH, COL_DESCRIPCION_WIDTH);
	_out("),\n");
	_emitReportRowsCTE(fromBuffer, toBuffer, true);
	_out(",\n");
	_out("paginadas AS (\n");
	_out("    -- Paginacion por LINEAS (no por filas): tras el wrap multi-linea,\n");
	_out("    -- una operacion larga puede ocupar varias lineas y queremos que\n");
	_out("    -- corten parejo al cambiar de pagina.\n");
	_out("    SELECT texto, rn, ((rn - 1) / %d + 1)::int AS pagina\n", PDF_LINES_PER_PAGE);
	_out("    FROM filas_alineadas\n");
	_out("),\n");
	_out("-- Bloque PDF con el balance del periodo. Se concatena al final del\n");
	_out("-- bloque BT/ET de Courier de la ULTIMA pagina de datos (o de la pagina\n");
	_out("-- vacia si no hubo operaciones). No abre BT/ET propio: se anexa al que\n");
	_out("-- la tabla dejo activo, usando T* para avanzar lineas con el mismo\n");
	_out("-- leading. La linea '() Tj T*' inicial es una linea en blanco entre la\n");
	_out("-- tabla y el balance.\n");
	_out("balance_pdf_block AS (\n");
	_out("    SELECT '() Tj T*' || chr(10) ||\n");
	_out("           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	_out("           '(Balance del periodo) Tj T*' || chr(10) ||\n");
	_out("           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	_out("           COALESCE((SELECT string_agg(\n");
	_out("               '(' || rpad(divisa, %d) || ' ingresos: ' || lpad(to_char(ingresos, 'FM9999999990.00'), 15) || ') Tj T*' || chr(10) ||\n", COL_DIVISA_WIDTH);
	_out("               '(' || rpad(divisa, %d) || ' egresos:  ' || lpad(to_char(egresos,  'FM9999999990.00'), 15) || ') Tj T*' || chr(10) ||\n", COL_DIVISA_WIDTH);
	_out("               '(' || rpad(divisa, %d) || ' balance:  ' || lpad(to_char(neto,     'FM9999999990.00'), 15) || ') Tj T*',\n", COL_DIVISA_WIDTH);
	_out("               chr(10) ORDER BY divisa) FROM balance),\n");
	_out("               '(Sin movimientos en el periodo.) Tj T*') AS bal\n");
	_out("),\n");
	_out("-- Body de cada pagina de DATOS: 3 bloques BT/ET para que cada uno use\n");
	_out("-- su fuente y posicion independientes:\n");
	_out("--   1) Titulo en Helvetica-Bold 14 (/F1) en Y=565.\n");
	_out("--   2) Periodo en Helvetica-Bold 10 (/F1) en Y=545.\n");
	_out("--   3) Tabla en Courier 9 (/F2) en Y=520 con leading 11.\n");
	_out("-- La ULTIMA pagina ademas anexa el balance dentro del mismo BT/ET de\n");
	_out("-- Courier (antes del ET final).\n");
	_out("paginas_data AS (\n");
	_out("    SELECT pagina,\n");
	_out("           'BT /F1 14 Tf 30 565 Td (Reporte de operaciones - pagina ' || pagina::text || ') Tj ET' || chr(10) ||\n");
	_out("           'BT /F1 10 Tf 30 545 Td (Periodo: %s a %s) Tj ET' || chr(10) ||\n", fromBuffer, toBuffer);
	_out("           'BT /F2 9 Tf 30 520 Td 11 TL' || chr(10) ||\n");
	_out("           '(' || (SELECT header_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	_out("           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	_out("           string_agg('(' || texto || ') Tj T*', chr(10) ORDER BY rn) || chr(10) ||\n");
	_out("           CASE WHEN pagina = (SELECT MAX(pagina) FROM paginadas)\n");
	_out("                THEN (SELECT bal FROM balance_pdf_block) || chr(10)\n");
	_out("                ELSE '' END ||\n");
	_out("           'ET' AS s\n");
	_out("    FROM paginadas\n");
	_out("    GROUP BY pagina\n");
	_out("),\n");
	_out("-- Si no hubo operaciones generamos una unica pagina con el mensaje y el\n");
	_out("-- balance (que en ese caso dice 'Sin movimientos en el periodo.').\n");
	_out("paginas AS (\n");
	_out("    SELECT pagina, s FROM paginas_data\n");
	_out("    UNION ALL\n");
	_out("    SELECT 1 AS pagina,\n");
	_out("           'BT /F1 14 Tf 30 565 Td (Reporte de operaciones) Tj ET' || chr(10) ||\n");
	_out("           'BT /F1 10 Tf 30 545 Td (Periodo: %s a %s) Tj ET' || chr(10) ||\n", fromBuffer, toBuffer);
	_out("           'BT /F2 9 Tf 30 520 Td 11 TL' || chr(10) ||\n");
	_out("           '(Sin operaciones en el periodo.) Tj T*' || chr(10) ||\n");
	_out("           (SELECT bal FROM balance_pdf_block) || chr(10) ||\n");
	_out("           'ET'\n");
	_out("    WHERE NOT EXISTS (SELECT 1 FROM paginas_data)\n");
	_out("),\n");
	_out("meta AS (\n");
	_out("    -- font1_num = primera fuente (Helvetica-Bold), font2_num = segunda (Courier).\n");
	_out("    SELECT COUNT(*)::int AS n,\n");
	_out("           (3 + 2 * COUNT(*))::int AS font1_num,\n");
	_out("           (4 + 2 * COUNT(*))::int AS font2_num\n");
	_out("    FROM paginas\n");
	_out("),\n");
	_out("-- Objetos numerados; el orden en el archivo coincide con el num.\n");
	_out("objetos AS (\n");
	_out("    SELECT 1 AS num,\n");
	_out("           '1 0 obj' || chr(10) || '<< /Type /Catalog /Pages 2 0 R >>' || chr(10) || 'endobj' AS body\n");
	_out("    UNION ALL\n");
	_out("    SELECT 2,\n");
	_out("           '2 0 obj' || chr(10) ||\n");
	_out("           '<< /Type /Pages /Kids [' ||\n");
	_out("           (SELECT string_agg((2 + p)::text || ' 0 R', ' ' ORDER BY p)\n");
	_out("            FROM generate_series(1, (SELECT n FROM meta)) AS gs(p)) ||\n");
	_out("           '] /Count ' || (SELECT n FROM meta)::text || ' >>' || chr(10) || 'endobj'\n");
	_out("    UNION ALL\n");
	_out("    SELECT (2 + gs.p)::int,\n");
	_out("           (2 + gs.p)::text || ' 0 obj' || chr(10) ||\n");
	_out("           '<< /Type /Page /Parent 2 0 R /MediaBox [0 0 842 595] ' ||\n");
	_out("           '/Resources << /Font << /F1 ' || (SELECT font1_num FROM meta)::text || ' 0 R ' ||\n");
	_out("           '/F2 ' || (SELECT font2_num FROM meta)::text || ' 0 R >> >> ' ||\n");
	_out("           '/Contents ' || (2 + (SELECT n FROM meta) + gs.p)::text || ' 0 R >>' ||\n");
	_out("           chr(10) || 'endobj'\n");
	_out("    FROM generate_series(1, (SELECT n FROM meta)) AS gs(p)\n");
	_out("    UNION ALL\n");
	_out("    SELECT (2 + (SELECT n FROM meta) + p.pagina)::int,\n");
	_out("           (2 + (SELECT n FROM meta) + p.pagina)::text || ' 0 obj' || chr(10) ||\n");
	_out("           '<< /Length ' || octet_length(p.s)::text || ' >>' || chr(10) ||\n");
	_out("           'stream' || chr(10) || p.s || chr(10) || 'endstream' || chr(10) || 'endobj'\n");
	_out("    FROM paginas p\n");
	_out("    UNION ALL\n");
	_out("    SELECT (SELECT font1_num FROM meta),\n");
	_out("           (SELECT font1_num FROM meta)::text || ' 0 obj' || chr(10) ||\n");
	_out("           '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>' || chr(10) || 'endobj'\n");
	_out("    UNION ALL\n");
	_out("    SELECT (SELECT font2_num FROM meta),\n");
	_out("           (SELECT font2_num FROM meta)::text || ' 0 obj' || chr(10) ||\n");
	_out("           '<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>' || chr(10) || 'endobj'\n");
	_out("),\n");
	_out("-- Offset acumulado de cada objeto en el archivo: 9 (header + LF) +\n");
	_out("-- octet_length() de los previos, contando un LF entre cada uno.\n");
	_out("con_offset AS (\n");
	_out("    SELECT num, body,\n");
	_out("           9 + COALESCE(SUM(octet_length(body) + 1) OVER (\n");
	_out("               ORDER BY num ROWS BETWEEN UNBOUNDED PRECEDING AND 1 PRECEDING\n");
	_out("           ), 0) AS off\n");
	_out("    FROM objetos\n");
	_out("),\n");
	_out("totales AS (\n");
	_out("    SELECT 9 + COALESCE(SUM(octet_length(body) + 1), 0) AS xref_off,\n");
	_out("           COUNT(*)::int + 1 AS size\n");
	_out("    FROM objetos\n");
	_out(")\n");
	_out("SELECT '%%PDF-1.4' || chr(10) ||\n");
	_out("       (SELECT string_agg(body, chr(10) ORDER BY num) FROM con_offset) || chr(10) ||\n");
	_out("       'xref' || chr(10) ||\n");
	_out("       '0 ' || (SELECT size FROM totales)::text || chr(10) ||\n");
	_out("       '0000000000 65535 f ' || chr(10) ||\n");
	_out("       (SELECT string_agg(to_char(off, 'FM0000000000') || ' 00000 n ', chr(10) ORDER BY num)\n");
	_out("        FROM con_offset) || chr(10) ||\n");
	_out("       'trailer' || chr(10) ||\n");
	_out("       '<< /Size ' || (SELECT size FROM totales)::text || ' /Root 1 0 R >>' || chr(10) ||\n");
	_out("       'startxref' || chr(10) ||\n");
	_out("       (SELECT xref_off FROM totales)::text || chr(10) ||\n");
	_out("       '%%%%EOF' AS reporte_pdf");
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
	_out("-- Auto-persistencia con psql: si se ejecuta con 'psql -f', el reporte\n");
	_out("--    se guarda en el CWD del cliente como\n");
	_out("--    'reporte_DD-MM-YYYY_HHh.MMm.SSs.%s'. Otros clientes rechazaran\n", extension);
	_out("--    los meta-comandos pero ya recibieron el contenido en (1).\n");
	_out("SELECT 'reporte_' || to_char(now(), 'DD-MM-YYYY\"_\"HH24\"h.\"MI\"m.\"SS\"s\"') || '.%s' AS fname \\gset\n", extension);
	_out("\\pset format unaligned\n");
	_out("\\pset tuples_only on\n");
	_out("\\pset recordsep ''\n");
	_out("\\o :fname\n");
	emitSelect(period);
	_out(";\n");
	_out("\\o\n");
	_out("\\pset format aligned\n");
	_out("\\pset tuples_only off\n");
	_out("\\pset recordsep '\\n'\n");
	_out("\\echo 'Reporte guardado en' :fname\n\n");
}

static void _generateReport(const ReportSentence * report) {
	const DatePeriod * period = report->period;
	const char * extension = NULL;
	ReportSelectEmitter emitSelect = NULL;

	switch (report->format->kind) {
		case REPORT_FORMAT_HTML:
			_out("-- Reporte (formato HTML: documento HTML completo armado en SQL).\n");
			extension = "html";
			emitSelect = _emitHtmlSelect;
			break;
		case REPORT_FORMAT_PLAIN_TEXT:
			_out("-- Reporte (formato texto plano: tabla ASCII alineada).\n");
			extension = "txt";
			emitSelect = _emitTextReportSelect;
			break;
		case REPORT_FORMAT_PDF:
			_out("-- Reporte (formato PDF: documento PDF 1.4 armado en SQL).\n");
			extension = "pdf";
			emitSelect = _emitPdfSelect;
			break;
		default:
			return;
	}

	// 1) SELECT puro: cualquier cliente recibe el contenido del reporte como
	//    una fila/columna. El PDF se devuelve como text; el HTML como un
	//    documento completo; el texto plano como una fila por operacion.
	_out("-- 1) SELECT puro: devuelve el contenido del reporte (cualquier cliente).\n");
	emitSelect(period);
	_out(";\n\n");

	// 2) Bloque de meta-comandos psql que copia el mismo SELECT a un archivo
	//    con timestamp en el CWD del cliente.
	_emitReportSavingBlock(extension, period, emitSelect);
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
