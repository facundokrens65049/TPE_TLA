#include "ReportModel.h"
#include "../../../support/io/EmitSql.h"
#include "../../../support/language/DateUtils.h"

// Resolves an AST Date node to a concrete DateValue (literal or reserved
// word such as 'hoy'/'ayer'/'manana').
static DateValue _resolveDate(const Date * date) {
	switch (date->kind) {
		case DATE_KIND_LITERAL:   return parseLiteralDate(date->literal);
		case DATE_KIND_TODAY:     return today();
		case DATE_KIND_YESTERDAY: return yesterday();
		case DATE_KIND_TOMORROW:  return tomorrow();
		default:                  return INVALID_DATE_VALUE;
	}
}

void resolvePeriodBounds(const DatePeriod * period, char * fromBuffer, char * toBuffer) {
	if (period->kind == DATE_PERIOD_RANGE) {
		formatDateValueIso(_resolveDate(period->fromDate), fromBuffer);
		formatDateValueIso(_resolveDate(period->toDate), toBuffer);
		return;
	}
	const DateValue close = today();
	DateValue start = close;
	switch (period->frequency->kind) {
		case FREQUENCY_MONTHLY: start = addMonths(close, -1); break;
		case FREQUENCY_WEEKLY:  start = addDays(close, -7); break;
		case FREQUENCY_YEARLY:  start = addMonths(close, -12); break;
		default:                start = addMonths(close, -1); break;
	}
	formatDateValueIso(start, fromBuffer);
	formatDateValueIso(close, toBuffer);
}

void emitReportHeaderCTE(void) {
	emitSql("cabecera AS (\n");
	emitSql("    SELECT\n");
	emitSql("        rpad('id', %d) || ' ' || rpad('tipo', %d) || ' ' || lpad('monto', %d) || ' ' ||\n",
		COL_ID_WIDTH, COL_TYPE_WIDTH, COL_AMOUNT_WIDTH);
	emitSql("        rpad('divisa', %d) || ' ' || rpad('categoria', %d) || ' ' || rpad('fecha', %d) || ' ' ||\n",
		COL_CURRENCY_WIDTH, COL_CATEGORY_WIDTH, COL_DATE_WIDTH);
	emitSql("        rpad('detalle', %d) || ' ' || 'descripcion' AS header_line,\n", COL_DETAIL_WIDTH);
	emitSql("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n",
		COL_ID_WIDTH, COL_TYPE_WIDTH, COL_AMOUNT_WIDTH);
	emitSql("        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n",
		COL_CURRENCY_WIDTH, COL_CATEGORY_WIDTH, COL_DATE_WIDTH);
	emitSql("        repeat('-', %d) || ' ' || repeat('-', %d) AS separator_line\n",
		COL_DETAIL_WIDTH, COL_DESCRIPTION_WIDTH);
	emitSql(")");
}

void emitReportRowsCTE(const char * fromBuffer, const char * toBuffer, bool pdfEscape) {
	emitSql("filas_base AS (\n");
	emitSql("    -- Gastos e ingresos puntuales.\n");
	emitSql("    SELECT id, tipo, monto, divisa, categoria, fecha, descripcion,\n");
	emitSql("           CAST(NULL AS text) AS detalle\n");
	emitSql("    FROM operaciones\n");
	emitSql("    WHERE tipo IN ('gasto', 'ingreso')\n");
	emitSql("      AND fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	emitSql("    UNION ALL\n");
	emitSql("    -- Cuotas individuales: una fila por cuota con su fecha real, monto\n");
	emitSql("    -- prorrateado y detalle 'k/N'. La fila padre de cuotas no aparece.\n");
	emitSql("    SELECT op.id, 'cuota'::text, c.monto, c.divisa, op.categoria,\n");
	emitSql("           c.fecha, op.descripcion,\n");
	emitSql("           c.numero_cuota::text || '/' || c.total_cuotas::text\n");
	emitSql("    FROM cuotas c JOIN operaciones op ON op.id = c.operacion_id\n");
	emitSql("    WHERE c.fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	emitSql("    UNION ALL\n");
	emitSql("    -- Suscripciones: la fila padre + frecuencia como detalle.\n");
	emitSql("    SELECT op.id, op.tipo, op.monto, op.divisa, op.categoria,\n");
	emitSql("           op.fecha, op.descripcion, s.frecuencia\n");
	emitSql("    FROM operaciones op JOIN suscripciones s ON s.operacion_id = op.id\n");
	emitSql("    WHERE op.tipo = 'suscripcion'\n");
	emitSql("      AND op.fecha BETWEEN DATE '%s' AND DATE '%s'\n", fromBuffer, toBuffer);
	emitSql("),\n");
	emitSql("filas_wrap AS (\n");
	emitSql("    -- Lineas necesarias para envolver categoria/descripcion sin cortar.\n");
	emitSql("    SELECT *,\n");
	emitSql("           GREATEST(\n");
	emitSql("               CEIL(GREATEST(length(COALESCE(categoria, '')), 1)::numeric / %d)::int,\n", COL_CATEGORY_WIDTH);
	emitSql("               CEIL(GREATEST(length(COALESCE(descripcion, '')), 1)::numeric / %d)::int,\n", COL_DESCRIPTION_WIDTH);
	emitSql("               1\n");
	emitSql("           ) AS line_count\n");
	emitSql("    FROM filas_base\n");
	emitSql("),\n");
	emitSql("filas_expandidas AS (\n");
	emitSql("    -- Una fila por linea visible (j = 0..line_count-1).\n");
	emitSql("    SELECT f.*, gs.n - 1 AS j\n");
	emitSql("    FROM filas_wrap f, generate_series(1, f.line_count) AS gs(n)\n");
	emitSql("),\n");
	emitSql("filas_lineas AS (\n");
	emitSql("    -- Composicion final: solo j=0 muestra columnas estaticas; categoria\n");
	emitSql("    -- y descripcion van wrap-eadas en ventanas de su ancho.\n");
	emitSql("    SELECT\n");
	emitSql("        CASE WHEN j = 0 THEN rpad(id::text, %d)         ELSE rpad('', %d) END || ' ' ||\n", COL_ID_WIDTH, COL_ID_WIDTH);
	emitSql("        CASE WHEN j = 0 THEN rpad(left(tipo, %d), %d)    ELSE rpad('', %d) END || ' ' ||\n", COL_TYPE_WIDTH, COL_TYPE_WIDTH, COL_TYPE_WIDTH);
	emitSql("        CASE WHEN j = 0 THEN lpad(monto::text, %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_AMOUNT_WIDTH, COL_AMOUNT_WIDTH);
	emitSql("        CASE WHEN j = 0 THEN rpad(left(divisa, %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_CURRENCY_WIDTH, COL_CURRENCY_WIDTH, COL_CURRENCY_WIDTH);
	emitSql("        rpad(COALESCE(substring(COALESCE(categoria, '') FROM j * %d + 1 FOR %d), ''), %d) || ' ' ||\n", COL_CATEGORY_WIDTH, COL_CATEGORY_WIDTH, COL_CATEGORY_WIDTH);
	emitSql("        CASE WHEN j = 0 THEN rpad(to_char(fecha, '" DSL_DATE_DISPLAY_MASK "'), %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_DATE_WIDTH, COL_DATE_WIDTH);
	emitSql("        CASE WHEN j = 0 THEN rpad(left(COALESCE(detalle, ''), %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_DETAIL_WIDTH, COL_DETAIL_WIDTH, COL_DETAIL_WIDTH);
	emitSql("        rpad(COALESCE(substring(COALESCE(descripcion, '') FROM j * %d + 1 FOR %d), ''), %d) AS linea_raw,\n", COL_DESCRIPTION_WIDTH, COL_DESCRIPTION_WIDTH, COL_DESCRIPTION_WIDTH);
	emitSql("        fecha, id, j\n");
	emitSql("    FROM filas_expandidas\n");
	emitSql(")");
	if (pdfEscape) {
		emitSql(",\n");
		emitSql("filas_alineadas AS (\n");
		emitSql("    -- Escape de los caracteres reservados de los string literals PDF.\n");
		emitSql("    SELECT replace(replace(replace(replace(replace(replace(\n");
		emitSql("        linea_raw,\n");
		emitSql("        chr(92), chr(92) || chr(92)),\n");
		emitSql("        '(',     chr(92) || '('),\n");
		emitSql("        ')',     chr(92) || ')'),\n");
		emitSql("        chr(10), chr(92) || 'n'),\n");
		emitSql("        chr(13), chr(92) || 'r'),\n");
		emitSql("        chr(9),  chr(92) || 't') AS texto,\n");
		emitSql("        ROW_NUMBER() OVER (ORDER BY fecha, id, j) AS rn\n");
		emitSql("    FROM filas_lineas\n");
		emitSql(")");
	}
	emitSql(",\n");
	emitSql("balance AS (\n");
	emitSql("    -- Balance neto del periodo agrupado por divisa: ingresos como aporte\n");
	emitSql("    -- positivo, todo lo demas (gastos, cuotas individuales, suscripciones)\n");
	emitSql("    -- como egreso. El balance es la suma EXACTA de los montos visibles en\n");
	emitSql("    -- la tabla (con sus signos), asi que cuadra con la columna 'monto'.\n");
	emitSql("    SELECT divisa,\n");
	emitSql("           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE 0 END) AS ingresos,\n");
	emitSql("           SUM(CASE WHEN tipo <> 'ingreso' THEN monto ELSE 0 END) AS egresos,\n");
	emitSql("           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE -monto END) AS neto\n");
	emitSql("    FROM filas_base\n");
	emitSql("    GROUP BY divisa\n");
	emitSql(")");
}
