#include "ReportModel.h"
#include <stdio.h>
#include <stdlib.h>

char * buildReportHeaderCTE(void) {
	char * buffer = NULL;
	size_t size = 0;
	FILE * out = open_memstream(&buffer, &size);
	fprintf(out, "cabecera AS (\n");
	fprintf(out, "    SELECT\n");
	fprintf(out, "        rpad('id', %d) || ' ' || rpad('tipo', %d) || ' ' || lpad('monto', %d) || ' ' ||\n",
		COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	fprintf(out, "        rpad('divisa', %d) || ' ' || rpad('categoria', %d) || ' ' || rpad('fecha', %d) || ' ' ||\n",
		COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	fprintf(out, "        rpad('detalle', %d) || ' ' || 'descripcion' AS header_line,\n", COL_DETALLE_WIDTH);
	fprintf(out, "        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n",
		COL_ID_WIDTH, COL_TIPO_WIDTH, COL_MONTO_WIDTH);
	fprintf(out, "        repeat('-', %d) || ' ' || repeat('-', %d) || ' ' || repeat('-', %d) || ' ' ||\n",
		COL_DIVISA_WIDTH, COL_CATEGORIA_WIDTH, COL_FECHA_WIDTH);
	fprintf(out, "        repeat('-', %d) || ' ' || repeat('-', %d) AS separator_line\n",
		COL_DETALLE_WIDTH, COL_DESCRIPCION_WIDTH);
	fprintf(out, ")");
	fclose(out);
	return buffer;
}

char * buildReportRowsCTE(const char * from, const char * to, bool pdfEscape) {
	char * buffer = NULL;
	size_t size = 0;
	FILE * out = open_memstream(&buffer, &size);
	fprintf(out, "filas_base AS (\n");
	fprintf(out, "    -- Gastos e ingresos puntuales.\n");
	fprintf(out, "    SELECT id, tipo, monto, divisa, categoria, fecha, descripcion,\n");
	fprintf(out, "           CAST(NULL AS text) AS detalle\n");
	fprintf(out, "    FROM operaciones\n");
	fprintf(out, "    WHERE tipo IN ('gasto', 'ingreso')\n");
	fprintf(out, "      AND fecha BETWEEN DATE '%s' AND DATE '%s'\n", from, to);
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    -- Cuotas individuales: una fila por cuota con su fecha real, monto\n");
	fprintf(out, "    -- prorrateado y detalle 'k/N'. La fila padre de cuotas no aparece.\n");
	fprintf(out, "    SELECT op.id, 'cuota'::text, c.monto, c.divisa, op.categoria,\n");
	fprintf(out, "           c.fecha, op.descripcion,\n");
	fprintf(out, "           c.numero_cuota::text || '/' || c.total_cuotas::text\n");
	fprintf(out, "    FROM cuotas c JOIN operaciones op ON op.id = c.operacion_id\n");
	fprintf(out, "    WHERE c.fecha BETWEEN DATE '%s' AND DATE '%s'\n", from, to);
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    -- Suscripciones: la fila padre + frecuencia como detalle.\n");
	fprintf(out, "    SELECT op.id, op.tipo, op.monto, op.divisa, op.categoria,\n");
	fprintf(out, "           op.fecha, op.descripcion, s.frecuencia\n");
	fprintf(out, "    FROM operaciones op JOIN suscripciones s ON s.operacion_id = op.id\n");
	fprintf(out, "    WHERE op.tipo = 'suscripcion'\n");
	fprintf(out, "      AND op.fecha BETWEEN DATE '%s' AND DATE '%s'\n", from, to);
	fprintf(out, "),\n");
	fprintf(out, "filas_wrap AS (\n");
	fprintf(out, "    -- Lineas necesarias para envolver categoria/descripcion sin cortar.\n");
	fprintf(out, "    SELECT *,\n");
	fprintf(out, "           GREATEST(\n");
	fprintf(out, "               CEIL(GREATEST(length(COALESCE(categoria, '')), 1)::numeric / %d)::int,\n", COL_CATEGORIA_WIDTH);
	fprintf(out, "               CEIL(GREATEST(length(COALESCE(descripcion, '')), 1)::numeric / %d)::int,\n", COL_DESCRIPCION_WIDTH);
	fprintf(out, "               1\n");
	fprintf(out, "           ) AS line_count\n");
	fprintf(out, "    FROM filas_base\n");
	fprintf(out, "),\n");
	fprintf(out, "filas_expandidas AS (\n");
	fprintf(out, "    -- Una fila por linea visible (j = 0..line_count-1).\n");
	fprintf(out, "    SELECT f.*, gs.n - 1 AS j\n");
	fprintf(out, "    FROM filas_wrap f, generate_series(1, f.line_count) AS gs(n)\n");
	fprintf(out, "),\n");
	fprintf(out, "filas_lineas AS (\n");
	fprintf(out, "    -- Composicion final: solo j=0 muestra columnas estaticas; categoria\n");
	fprintf(out, "    -- y descripcion van wrap-eadas en ventanas de su ancho.\n");
	fprintf(out, "    SELECT\n");
	fprintf(out, "        CASE WHEN j = 0 THEN rpad(id::text, %d)         ELSE rpad('', %d) END || ' ' ||\n", COL_ID_WIDTH, COL_ID_WIDTH);
	fprintf(out, "        CASE WHEN j = 0 THEN rpad(left(tipo, %d), %d)    ELSE rpad('', %d) END || ' ' ||\n", COL_TIPO_WIDTH, COL_TIPO_WIDTH, COL_TIPO_WIDTH);
	fprintf(out, "        CASE WHEN j = 0 THEN lpad(monto::text, %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_MONTO_WIDTH, COL_MONTO_WIDTH);
	fprintf(out, "        CASE WHEN j = 0 THEN rpad(left(divisa, %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_DIVISA_WIDTH, COL_DIVISA_WIDTH, COL_DIVISA_WIDTH);
	fprintf(out, "        rpad(COALESCE(substring(COALESCE(categoria, '') FROM j * %d + 1 FOR %d), ''), %d) || ' ' ||\n", COL_CATEGORIA_WIDTH, COL_CATEGORIA_WIDTH, COL_CATEGORIA_WIDTH);
	fprintf(out, "        CASE WHEN j = 0 THEN rpad(fecha::text, %d)      ELSE rpad('', %d) END || ' ' ||\n", COL_FECHA_WIDTH, COL_FECHA_WIDTH);
	fprintf(out, "        CASE WHEN j = 0 THEN rpad(left(COALESCE(detalle, ''), %d), %d) ELSE rpad('', %d) END || ' ' ||\n", COL_DETALLE_WIDTH, COL_DETALLE_WIDTH, COL_DETALLE_WIDTH);
	fprintf(out, "        rpad(COALESCE(substring(COALESCE(descripcion, '') FROM j * %d + 1 FOR %d), ''), %d) AS linea_raw,\n", COL_DESCRIPCION_WIDTH, COL_DESCRIPCION_WIDTH, COL_DESCRIPCION_WIDTH);
	fprintf(out, "        fecha, id, j\n");
	fprintf(out, "    FROM filas_expandidas\n");
	fprintf(out, ")");
	if (pdfEscape) {
		fprintf(out, ",\n");
		fprintf(out, "filas_alineadas AS (\n");
		fprintf(out, "    -- Escape de los caracteres reservados de los string literals PDF.\n");
		fprintf(out, "    SELECT replace(replace(replace(replace(replace(replace(\n");
		fprintf(out, "        linea_raw,\n");
		fprintf(out, "        chr(92), chr(92) || chr(92)),\n");
		fprintf(out, "        '(',     chr(92) || '('),\n");
		fprintf(out, "        ')',     chr(92) || ')'),\n");
		fprintf(out, "        chr(10), chr(92) || 'n'),\n");
		fprintf(out, "        chr(13), chr(92) || 'r'),\n");
		fprintf(out, "        chr(9),  chr(92) || 't') AS texto,\n");
		fprintf(out, "        ROW_NUMBER() OVER (ORDER BY fecha, id, j) AS rn\n");
		fprintf(out, "    FROM filas_lineas\n");
		fprintf(out, ")");
	}
	fprintf(out, ",\n");
	fprintf(out, "balance AS (\n");
	fprintf(out, "    -- Balance neto del periodo agrupado por divisa: ingresos como aporte\n");
	fprintf(out, "    -- positivo, todo lo demas (gastos, cuotas individuales, suscripciones)\n");
	fprintf(out, "    -- como egreso. El balance es la suma EXACTA de los montos visibles en\n");
	fprintf(out, "    -- la tabla (con sus signos), asi que cuadra con la columna 'monto'.\n");
	fprintf(out, "    SELECT divisa,\n");
	fprintf(out, "           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE 0 END) AS ingresos,\n");
	fprintf(out, "           SUM(CASE WHEN tipo <> 'ingreso' THEN monto ELSE 0 END) AS egresos,\n");
	fprintf(out, "           SUM(CASE WHEN tipo = 'ingreso' THEN monto ELSE -monto END) AS neto\n");
	fprintf(out, "    FROM filas_base\n");
	fprintf(out, "    GROUP BY divisa\n");
	fprintf(out, ")");
	fclose(out);
	return buffer;
}
