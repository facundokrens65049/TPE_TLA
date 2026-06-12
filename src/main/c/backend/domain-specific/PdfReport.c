#include "PdfReport.h"
#include "ReportModel.h"
#include <stdio.h>
#include <stdlib.h>

// Arma un PDF 1.4 paginado (fuente Courier, hasta N filas por pagina) en SQL
// puro. Devuelve una sola fila con el documento completo como text.

#define PDF_LINES_PER_PAGE 30

char * buildPdfReportSelect(const char * from, const char * to) {
	char * buffer = NULL;
	size_t size = 0;
	FILE * out = open_memstream(&buffer, &size);

	char * headerCTE = buildReportHeaderCTE();
	char * rowsCTE = buildReportRowsCTE(from, to, true);

	fprintf(out, "WITH\n");
	fprintf(out, "%s", headerCTE);
	fprintf(out, ",\n");
	fprintf(out, "%s", rowsCTE);
	fprintf(out, ",\n");
	fprintf(out, "paginadas AS (\n");
	fprintf(out, "    -- Paginacion por LINEAS (no por filas): tras el wrap multi-linea,\n");
	fprintf(out, "    -- una operacion larga puede ocupar varias lineas y queremos que\n");
	fprintf(out, "    -- corten parejo al cambiar de pagina.\n");
	fprintf(out, "    SELECT texto, rn, ((rn - 1) / %d + 1)::int AS pagina\n", PDF_LINES_PER_PAGE);
	fprintf(out, "    FROM filas_alineadas\n");
	fprintf(out, "),\n");
	fprintf(out, "-- Bloque PDF con el balance del periodo. Se concatena al final del\n");
	fprintf(out, "-- bloque BT/ET de Courier de la ULTIMA pagina de datos (o de la pagina\n");
	fprintf(out, "-- vacia si no hubo operaciones). No abre BT/ET propio: se anexa al que\n");
	fprintf(out, "-- la tabla dejo activo, usando T* para avanzar lineas con el mismo\n");
	fprintf(out, "-- leading. La linea '() Tj T*' inicial es una linea en blanco entre la\n");
	fprintf(out, "-- tabla y el balance.\n");
	fprintf(out, "balance_pdf_block AS (\n");
	fprintf(out, "    SELECT '() Tj T*' || chr(10) ||\n");
	fprintf(out, "           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	fprintf(out, "           '(Balance del periodo) Tj T*' || chr(10) ||\n");
	fprintf(out, "           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	fprintf(out, "           COALESCE((SELECT string_agg(\n");
	fprintf(out, "               '(' || rpad(divisa, %d) || ' ingresos: ' || lpad(to_char(ingresos, 'FM9999999990.00'), 15) || ') Tj T*' || chr(10) ||\n", COL_DIVISA_WIDTH);
	fprintf(out, "               '(' || rpad(divisa, %d) || ' egresos:  ' || lpad(to_char(egresos,  'FM9999999990.00'), 15) || ') Tj T*' || chr(10) ||\n", COL_DIVISA_WIDTH);
	fprintf(out, "               '(' || rpad(divisa, %d) || ' balance:  ' || lpad(to_char(neto,     'FM9999999990.00'), 15) || ') Tj T*',\n", COL_DIVISA_WIDTH);
	fprintf(out, "               chr(10) ORDER BY divisa) FROM balance),\n");
	fprintf(out, "               '(Sin movimientos en el periodo.) Tj T*') AS bal\n");
	fprintf(out, "),\n");
	fprintf(out, "-- Body de cada pagina de DATOS: 3 bloques BT/ET para que cada uno use\n");
	fprintf(out, "-- su fuente y posicion independientes:\n");
	fprintf(out, "--   1) Titulo en Helvetica-Bold 14 (/F1) en Y=565.\n");
	fprintf(out, "--   2) Periodo en Helvetica-Bold 10 (/F1) en Y=545.\n");
	fprintf(out, "--   3) Tabla en Courier 9 (/F2) en Y=520 con leading 11.\n");
	fprintf(out, "-- La ULTIMA pagina ademas anexa el balance dentro del mismo BT/ET de\n");
	fprintf(out, "-- Courier (antes del ET final).\n");
	fprintf(out, "paginas_data AS (\n");
	fprintf(out, "    SELECT pagina,\n");
	fprintf(out, "           'BT /F1 14 Tf 30 565 Td (Reporte de operaciones - pagina ' || pagina::text || ') Tj ET' || chr(10) ||\n");
	fprintf(out, "           'BT /F1 10 Tf 30 545 Td (Periodo: %s a %s) Tj ET' || chr(10) ||\n", from, to);
	fprintf(out, "           'BT /F2 9 Tf 30 520 Td 11 TL' || chr(10) ||\n");
	fprintf(out, "           '(' || (SELECT header_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	fprintf(out, "           '(' || (SELECT separator_line FROM cabecera) || ') Tj T*' || chr(10) ||\n");
	fprintf(out, "           string_agg('(' || texto || ') Tj T*', chr(10) ORDER BY rn) || chr(10) ||\n");
	fprintf(out, "           CASE WHEN pagina = (SELECT MAX(pagina) FROM paginadas)\n");
	fprintf(out, "                THEN (SELECT bal FROM balance_pdf_block) || chr(10)\n");
	fprintf(out, "                ELSE '' END ||\n");
	fprintf(out, "           'ET' AS s\n");
	fprintf(out, "    FROM paginadas\n");
	fprintf(out, "    GROUP BY pagina\n");
	fprintf(out, "),\n");
	fprintf(out, "-- Si no hubo operaciones generamos una unica pagina con el mensaje y el\n");
	fprintf(out, "-- balance (que en ese caso dice 'Sin movimientos en el periodo.').\n");
	fprintf(out, "paginas AS (\n");
	fprintf(out, "    SELECT pagina, s FROM paginas_data\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT 1 AS pagina,\n");
	fprintf(out, "           'BT /F1 14 Tf 30 565 Td (Reporte de operaciones) Tj ET' || chr(10) ||\n");
	fprintf(out, "           'BT /F1 10 Tf 30 545 Td (Periodo: %s a %s) Tj ET' || chr(10) ||\n", from, to);
	fprintf(out, "           'BT /F2 9 Tf 30 520 Td 11 TL' || chr(10) ||\n");
	fprintf(out, "           '(Sin operaciones en el periodo.) Tj T*' || chr(10) ||\n");
	fprintf(out, "           (SELECT bal FROM balance_pdf_block) || chr(10) ||\n");
	fprintf(out, "           'ET'\n");
	fprintf(out, "    WHERE NOT EXISTS (SELECT 1 FROM paginas_data)\n");
	fprintf(out, "),\n");
	fprintf(out, "meta AS (\n");
	fprintf(out, "    -- font1_num = primera fuente (Helvetica-Bold), font2_num = segunda (Courier).\n");
	fprintf(out, "    SELECT COUNT(*)::int AS n,\n");
	fprintf(out, "           (3 + 2 * COUNT(*))::int AS font1_num,\n");
	fprintf(out, "           (4 + 2 * COUNT(*))::int AS font2_num\n");
	fprintf(out, "    FROM paginas\n");
	fprintf(out, "),\n");
	fprintf(out, "-- Objetos numerados; el orden en el archivo coincide con el num.\n");
	fprintf(out, "objetos AS (\n");
	fprintf(out, "    SELECT 1 AS num,\n");
	fprintf(out, "           '1 0 obj' || chr(10) || '<< /Type /Catalog /Pages 2 0 R >>' || chr(10) || 'endobj' AS body\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT 2,\n");
	fprintf(out, "           '2 0 obj' || chr(10) ||\n");
	fprintf(out, "           '<< /Type /Pages /Kids [' ||\n");
	fprintf(out, "           (SELECT string_agg((2 + p)::text || ' 0 R', ' ' ORDER BY p)\n");
	fprintf(out, "            FROM generate_series(1, (SELECT n FROM meta)) AS gs(p)) ||\n");
	fprintf(out, "           '] /Count ' || (SELECT n FROM meta)::text || ' >>' || chr(10) || 'endobj'\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT (2 + gs.p)::int,\n");
	fprintf(out, "           (2 + gs.p)::text || ' 0 obj' || chr(10) ||\n");
	fprintf(out, "           '<< /Type /Page /Parent 2 0 R /MediaBox [0 0 842 595] ' ||\n");
	fprintf(out, "           '/Resources << /Font << /F1 ' || (SELECT font1_num FROM meta)::text || ' 0 R ' ||\n");
	fprintf(out, "           '/F2 ' || (SELECT font2_num FROM meta)::text || ' 0 R >> >> ' ||\n");
	fprintf(out, "           '/Contents ' || (2 + (SELECT n FROM meta) + gs.p)::text || ' 0 R >>' ||\n");
	fprintf(out, "           chr(10) || 'endobj'\n");
	fprintf(out, "    FROM generate_series(1, (SELECT n FROM meta)) AS gs(p)\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT (2 + (SELECT n FROM meta) + p.pagina)::int,\n");
	fprintf(out, "           (2 + (SELECT n FROM meta) + p.pagina)::text || ' 0 obj' || chr(10) ||\n");
	fprintf(out, "           '<< /Length ' || octet_length(p.s)::text || ' >>' || chr(10) ||\n");
	fprintf(out, "           'stream' || chr(10) || p.s || chr(10) || 'endstream' || chr(10) || 'endobj'\n");
	fprintf(out, "    FROM paginas p\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT (SELECT font1_num FROM meta),\n");
	fprintf(out, "           (SELECT font1_num FROM meta)::text || ' 0 obj' || chr(10) ||\n");
	fprintf(out, "           '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>' || chr(10) || 'endobj'\n");
	fprintf(out, "    UNION ALL\n");
	fprintf(out, "    SELECT (SELECT font2_num FROM meta),\n");
	fprintf(out, "           (SELECT font2_num FROM meta)::text || ' 0 obj' || chr(10) ||\n");
	fprintf(out, "           '<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>' || chr(10) || 'endobj'\n");
	fprintf(out, "),\n");
	fprintf(out, "-- Offset acumulado de cada objeto en el archivo: 9 (header + LF) +\n");
	fprintf(out, "-- octet_length() de los previos, contando un LF entre cada uno.\n");
	fprintf(out, "con_offset AS (\n");
	fprintf(out, "    SELECT num, body,\n");
	fprintf(out, "           9 + COALESCE(SUM(octet_length(body) + 1) OVER (\n");
	fprintf(out, "               ORDER BY num ROWS BETWEEN UNBOUNDED PRECEDING AND 1 PRECEDING\n");
	fprintf(out, "           ), 0) AS off\n");
	fprintf(out, "    FROM objetos\n");
	fprintf(out, "),\n");
	fprintf(out, "totales AS (\n");
	fprintf(out, "    SELECT 9 + COALESCE(SUM(octet_length(body) + 1), 0) AS xref_off,\n");
	fprintf(out, "           COUNT(*)::int + 1 AS size\n");
	fprintf(out, "    FROM objetos\n");
	fprintf(out, ")\n");
	fprintf(out, "SELECT '%%PDF-1.4' || chr(10) ||\n");
	fprintf(out, "       (SELECT string_agg(body, chr(10) ORDER BY num) FROM con_offset) || chr(10) ||\n");
	fprintf(out, "       'xref' || chr(10) ||\n");
	fprintf(out, "       '0 ' || (SELECT size FROM totales)::text || chr(10) ||\n");
	fprintf(out, "       '0000000000 65535 f ' || chr(10) ||\n");
	fprintf(out, "       (SELECT string_agg(to_char(off, 'FM0000000000') || ' 00000 n ', chr(10) ORDER BY num)\n");
	fprintf(out, "        FROM con_offset) || chr(10) ||\n");
	fprintf(out, "       'trailer' || chr(10) ||\n");
	fprintf(out, "       '<< /Size ' || (SELECT size FROM totales)::text || ' /Root 1 0 R >>' || chr(10) ||\n");
	fprintf(out, "       'startxref' || chr(10) ||\n");
	fprintf(out, "       (SELECT xref_off FROM totales)::text || chr(10) ||\n");
	fprintf(out, "       '%%%%EOF' AS reporte_pdf");

	free(headerCTE);
	free(rowsCTE);
	fclose(out);
	return buffer;
}
