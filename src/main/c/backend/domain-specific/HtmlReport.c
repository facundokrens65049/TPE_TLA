#include "HtmlReport.h"
#include "ReportModel.h"
#include <stdio.h>
#include <stdlib.h>

char * buildHtmlReportSelect(const char * from, const char * to) {
	char * buffer = NULL;
	size_t size = 0;
	FILE * out = open_memstream(&buffer, &size);

	char * rowsCTE = buildReportRowsCTE(from, to, false);

	fprintf(out, "WITH\n");
	fprintf(out, "%s", rowsCTE);
	fprintf(out, "\n");
	fprintf(out, "SELECT '<!DOCTYPE html>'\n");
	fprintf(out, "    || '<html lang=\"es\"><head><meta charset=\"utf-8\">'\n");
	fprintf(out, "    || '<title>Reporte de operaciones</title>'\n");
	fprintf(out, "    || '<style>body{font-family:system-ui,sans-serif;margin:24px;color:#222}'\n");
	fprintf(out, "    || 'h1{font-size:18px;margin:0 0 4px}h2{font-size:15px;margin:24px 0 8px}'\n");
	fprintf(out, "    || 'p{margin:0 0 16px;color:#666}'\n");
	fprintf(out, "    || 'table{border-collapse:collapse;width:100%%;font-size:14px}'\n");
	fprintf(out, "    || 'th,td{border:1px solid #ddd;padding:6px 10px;text-align:left;vertical-align:top}'\n");
	fprintf(out, "    || 'th{background:#f3f4f6}tr:nth-child(even) td{background:#fafafa}'\n");
	fprintf(out, "    || 'td.num{text-align:right;font-variant-numeric:tabular-nums}'\n");
	fprintf(out, "    || 'td.det{white-space:nowrap;color:#555}'\n");
	fprintf(out, "    || 'table.balance{width:auto;min-width:380px}'\n");
	fprintf(out, "    || 'table.balance tr.neto td{font-weight:bold;background:#eef6ff}'\n");
	fprintf(out, "    || 'td.neg{color:#b00020}td.pos{color:#0a7a2a}'\n");
	fprintf(out, "    || '</style></head><body>'\n");
	fprintf(out, "    || '<h1>Reporte de operaciones</h1>'\n");
	fprintf(out, "    || '<p>Periodo: %s a %s</p>'\n", from, to);
	fprintf(out, "    || '<table>'\n");
	fprintf(out, "    || '<thead><tr>'\n");
	fprintf(out, "    || '<th>id</th><th>tipo</th><th>monto</th><th>divisa</th>'\n");
	fprintf(out, "    || '<th>categoria</th><th>fecha</th><th>detalle</th><th>descripcion</th>'\n");
	fprintf(out, "    || '</tr></thead><tbody>'\n");
	fprintf(out, "    || COALESCE((SELECT string_agg(\n");
	fprintf(out, "        '<tr>'\n");
	fprintf(out, "        || '<td class=\"num\">' || id::text || '</td>'\n");
	fprintf(out, "        || '<td>' || tipo || '</td>'\n");
	fprintf(out, "        || '<td class=\"num\">' || monto::text || '</td>'\n");
	fprintf(out, "        || '<td>' || divisa || '</td>'\n");
	fprintf(out, "        || '<td>' || COALESCE(categoria, '') || '</td>'\n");
	fprintf(out, "        || '<td>' || fecha::text || '</td>'\n");
	fprintf(out, "        || '<td class=\"det\">' || COALESCE(detalle, '') || '</td>'\n");
	fprintf(out, "        || '<td>' || COALESCE(descripcion, '') || '</td>'\n");
	fprintf(out, "        || '</tr>', '' ORDER BY fecha, id) FROM filas_base),\n");
	fprintf(out, "        '<tr><td colspan=\"8\" style=\"text-align:center;color:#888\">Sin operaciones en el periodo.</td></tr>'\n");
	fprintf(out, "    )\n");
	fprintf(out, "    || '</tbody></table>'\n");
	fprintf(out, "    || '<h2>Balance del periodo</h2>'\n");
	fprintf(out, "    || COALESCE((SELECT '<table class=\"balance\"><thead><tr>'\n");
	fprintf(out, "        || '<th>divisa</th><th>ingresos</th><th>egresos</th><th>balance neto</th>'\n");
	fprintf(out, "        || '</tr></thead><tbody>'\n");
	fprintf(out, "        || string_agg(\n");
	fprintf(out, "            '<tr class=\"neto\">'\n");
	fprintf(out, "            || '<td>' || divisa || '</td>'\n");
	fprintf(out, "            || '<td class=\"num pos\">' || to_char(ingresos, 'FM9999999990.00') || '</td>'\n");
	fprintf(out, "            || '<td class=\"num neg\">' || to_char(egresos,  'FM9999999990.00') || '</td>'\n");
	fprintf(out, "            || '<td class=\"num ' || CASE WHEN neto >= 0 THEN 'pos' ELSE 'neg' END || '\">'\n");
	fprintf(out, "            || to_char(neto, 'FM9999999990.00') || '</td>'\n");
	fprintf(out, "            || '</tr>', '' ORDER BY divisa)\n");
	fprintf(out, "        || '</tbody></table>' FROM balance),\n");
	fprintf(out, "        '<p style=\"color:#888\">Sin movimientos en el periodo.</p>'\n");
	fprintf(out, "    )\n");
	fprintf(out, "    || '</body></html>' || chr(10) AS reporte_html");

	free(rowsCTE);
	fclose(out);
	return buffer;
}
