#include "TextReport.h"
#include "ReportModel.h"
#include "../../support/io/EmitSql.h"

void emitTextReportSelect(const DatePeriod * period) {
	char fromBuffer[11];
	char toBuffer[11];
	resolvePeriodBounds(period, fromBuffer, toBuffer);

	emitSql("WITH\n");
	emitReportRowsCTE(fromBuffer, toBuffer, false);
	emitSql(",\n");
	emitReportHeaderCTE();
	emitSql("\n");
	emitSql("SELECT 'Reporte de operaciones' || chr(10) ||\n");
	emitSql("       'Periodo: %s a %s' || chr(10) || chr(10) ||\n", fromBuffer, toBuffer);
	emitSql("       (SELECT header_line FROM cabecera) || chr(10) ||\n");
	emitSql("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	emitSql("       COALESCE(\n");
	emitSql("           (SELECT string_agg(linea_raw, chr(10) ORDER BY fecha, id, j) FROM filas_lineas),\n");
	emitSql("           '(sin operaciones en el periodo)'\n");
	emitSql("       ) || chr(10) || chr(10) ||\n");
	emitSql("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	emitSql("       'Balance del periodo' || chr(10) ||\n");
	emitSql("       (SELECT separator_line FROM cabecera) || chr(10) ||\n");
	emitSql("       COALESCE(\n");
	emitSql("           (SELECT string_agg(\n");
	emitSql("               rpad(divisa, %d) || ' ingresos: ' || lpad(to_char(ingresos, 'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	emitSql("               rpad(divisa, %d) || ' egresos:  ' || lpad(to_char(egresos,  'FM9999999990.00'), 15) || chr(10) ||\n", COL_DIVISA_WIDTH);
	emitSql("               rpad(divisa, %d) || ' balance:  ' || lpad(to_char(neto,     'FM9999999990.00'), 15),\n", COL_DIVISA_WIDTH);
	emitSql("               chr(10) ORDER BY divisa\n");
	emitSql("           ) FROM balance),\n");
	emitSql("           '(sin movimientos en el periodo)'\n");
	emitSql("       ) || chr(10) AS reporte_texto");
}
