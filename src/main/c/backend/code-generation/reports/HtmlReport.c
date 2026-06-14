#include "HtmlReport.h"
#include "ReportModel.h"
#include "../../../support/io/EmitSql.h"
#include "../../../support/language/DateUtils.h"

// Emits the SQL expression that escapes HTML reserved characters on "expr"
// ('&' first, so we don't re-escape the entities the other replaces produce).
// The content comes from the database (category, description, currency,
// etc.) and is inserted raw into the document, so a '<', '>' or '&' would
// break the HTML or allow injection. The PDF already did its own escape;
// this gives the HTML the equivalent.
static void _emitHtmlEscaped(const char * expr) {
	emitSql("replace(replace(replace(%s, '&', '&amp;'), '<', '&lt;'), '>', '&gt;')", expr);
}

void emitHtmlReportSelect(const DatePeriod * period) {
	char fromBuffer[ISO_DATE_BUFFER_SIZE];
	char toBuffer[ISO_DATE_BUFFER_SIZE];
	resolvePeriodBounds(period, fromBuffer, toBuffer);

	// SQL DATE literals require ISO "YYYY-MM-DD"; for display we use the DSL's
	// "DD-MM-YYYY" so the report header matches the input format.
	char fromDisplay[ISO_DATE_BUFFER_SIZE];
	char toDisplay[ISO_DATE_BUFFER_SIZE];
	formatIsoAsDmy(fromBuffer, fromDisplay);
	formatIsoAsDmy(toBuffer, toDisplay);

	emitSql("WITH\n");
	emitReportRowsCTE(fromBuffer, toBuffer, false);
	emitSql("\n");
	emitSql("SELECT '<!DOCTYPE html>'\n");
	emitSql("    || '<html lang=\"es\"><head><meta charset=\"utf-8\">'\n");
	emitSql("    || '<title>Reporte de operaciones</title>'\n");
	emitSql("    || '<style>body{font-family:system-ui,sans-serif;margin:24px;color:#222}'\n");
	emitSql("    || 'h1{font-size:18px;margin:0 0 4px}h2{font-size:15px;margin:24px 0 8px}'\n");
	emitSql("    || 'p{margin:0 0 16px;color:#666}'\n");
	emitSql("    || 'table{border-collapse:collapse;width:100%%;font-size:14px}'\n");
	emitSql("    || 'th,td{border:1px solid #ddd;padding:6px 10px;text-align:left;vertical-align:top}'\n");
	emitSql("    || 'th{background:#f3f4f6}tr:nth-child(even) td{background:#fafafa}'\n");
	emitSql("    || 'td.num{text-align:right;font-variant-numeric:tabular-nums}'\n");
	emitSql("    || 'td.det{white-space:nowrap;color:#555}'\n");
	emitSql("    || 'table.balance{width:auto;min-width:380px}'\n");
	emitSql("    || 'table.balance tr.neto td{font-weight:bold;background:#eef6ff}'\n");
	emitSql("    || 'td.neg{color:#b00020}td.pos{color:#0a7a2a}'\n");
	emitSql("    || '</style></head><body>'\n");
	emitSql("    || '<h1>Reporte de operaciones</h1>'\n");
	emitSql("    || '<p>Periodo: %s a %s</p>'\n", fromDisplay, toDisplay);
	emitSql("    || '<table>'\n");
	emitSql("    || '<thead><tr>'\n");
	emitSql("    || '<th>id</th><th>tipo</th><th>monto</th><th>divisa</th>'\n");
	emitSql("    || '<th>categoria</th><th>fecha</th><th>detalle</th><th>descripcion</th>'\n");
	emitSql("    || '</tr></thead><tbody>'\n");
	emitSql("    || COALESCE((SELECT string_agg(\n");
	emitSql("        '<tr>'\n");
	emitSql("        || '<td class=\"num\">' || id::text || '</td>'\n");
	emitSql("        || '<td>' || ");
	_emitHtmlEscaped("tipo");
	emitSql(" || '</td>'\n");
	emitSql("        || '<td class=\"num\">' || monto::text || '</td>'\n");
	emitSql("        || '<td>' || ");
	_emitHtmlEscaped("divisa");
	emitSql(" || '</td>'\n");
	emitSql("        || '<td>' || COALESCE(");
	_emitHtmlEscaped("categoria");
	emitSql(", '') || '</td>'\n");
	emitSql("        || '<td>' || to_char(fecha, '" DSL_DATE_DISPLAY_MASK "') || '</td>'\n");
	emitSql("        || '<td class=\"det\">' || COALESCE(");
	_emitHtmlEscaped("detalle");
	emitSql(", '') || '</td>'\n");
	emitSql("        || '<td>' || COALESCE(");
	_emitHtmlEscaped("descripcion");
	emitSql(", '') || '</td>'\n");
	emitSql("        || '</tr>', '' ORDER BY fecha, id) FROM filas_base),\n");
	emitSql("        '<tr><td colspan=\"8\" style=\"text-align:center;color:#888\">Sin operaciones en el periodo.</td></tr>'\n");
	emitSql("    )\n");
	emitSql("    || '</tbody></table>'\n");
	emitSql("    || '<h2>Balance del periodo</h2>'\n");
	emitSql("    || COALESCE((SELECT '<table class=\"balance\"><thead><tr>'\n");
	emitSql("        || '<th>divisa</th><th>ingresos</th><th>egresos</th><th>balance neto</th>'\n");
	emitSql("        || '</tr></thead><tbody>'\n");
	emitSql("        || string_agg(\n");
	emitSql("            '<tr class=\"neto\">'\n");
	emitSql("            || '<td>' || ");
	_emitHtmlEscaped("divisa");
	emitSql(" || '</td>'\n");
	emitSql("            || '<td class=\"num pos\">' || to_char(ingresos, 'FM9999999990.00') || '</td>'\n");
	emitSql("            || '<td class=\"num neg\">' || to_char(egresos,  'FM9999999990.00') || '</td>'\n");
	emitSql("            || '<td class=\"num ' || CASE WHEN neto >= 0 THEN 'pos' ELSE 'neg' END || '\">'\n");
	emitSql("            || to_char(neto, 'FM9999999990.00') || '</td>'\n");
	emitSql("            || '</tr>', '' ORDER BY divisa)\n");
	emitSql("        || '</tbody></table>' FROM balance),\n");
	emitSql("        '<p style=\"color:#888\">Sin movimientos en el periodo.</p>'\n");
	emitSql("    )\n");
	emitSql("    || '</body></html>' || chr(10) AS reporte_html");
}
