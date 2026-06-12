#include "TextReport.h"
#include "ReportModel.h"
#include <stdio.h>
#include <stdlib.h>

char * buildTextReportSelect(const char * from, const char * to) {
	char * buffer = NULL;
	size_t size = 0;
	FILE * out = open_memstream(&buffer, &size);

	char * rowsCTE = buildReportRowsCTE(from, to, false);
	char * headerCTE = buildReportHeaderCTE();

	fprintf(out, "WITH\n");
	fprintf(out, "%s", rowsCTE);
	fprintf(out, ",\n");
	fprintf(out, "%s", headerCTE);
	fprintf(out, "\n");
	fprintf(out, "SELECT 'Reporte de operaciones' || chr(10) ||\n");
	fprintf(out, "       'Periodo: %s a %s' || chr(10) || chr(10) ||\n", from, to);
	fprintf(out, "       (SELECT header_line FROM cabecera) || chr(10) ||\n");
	fprintf(out, "       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	fprintf(out, "       COALESCE(\n");
	fprintf(out, "           (SELECT string_agg(linea_raw, chr(10) ORDER BY fecha, id, j) FROM filas_lineas),\n");
	fprintf(out, "           '(sin operaciones en el periodo)'\n");
	fprintf(out, "       ) || chr(10) || chr(10) ||\n");
	fprintf(out, "       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	fprintf(out, "       'Balance del periodo' || chr(10) ||\n");
	fprintf(out, "       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	fprintf(out, "       COALESCE(\n");
	fprintf(out, "           (SELECT string_agg(\n");
	fprintf(out, "               rpad(divisa, %d) || ' ingresos: ' || lpad(to_char(ingresos, 'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	fprintf(out, "               rpad(divisa, %d) || ' egresos:  ' || lpad(to_char(egresos,  'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	fprintf(out, "               rpad(divisa, %d) || ' balance:  ' || lpad(to_char(neto,     'FM9999999990.00'), 15),\n", COL_DIVISA_WIDTH);
	fprintf(out, "               chr(10) ORDER BY divisa\n");
	fprintf(out, "           ) FROM balance),\n");
	fprintf(out, "           '(sin movimientos en el periodo)'\n");
	fprintf(out, "       ) || chr(10) AS reporte_texto");

	free(rowsCTE);
	free(headerCTE);
	fclose(out);
	return buffer;
}
