#ifndef PDF_REPORT_HEADER
#define PDF_REPORT_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emite un SELECT (sin ';\n' final) que devuelve UNA SOLA fila con un
// documento PDF 1.4 (hoja A4 horizontal, fuentes base 14 Helvetica-Bold y
// Courier) armado enteramente en SQL puro -- sin extensiones ni librerias
// externas. La paginacion se hace por LINEAS (no filas) sobre el modelo
// compartido con texto plano; el balance se anexa al pie de la ultima
// pagina, dentro del mismo bloque BT/ET de la tabla. Reportes vacios
// generan una unica pagina con 'Sin operaciones' + 'Sin movimientos'.
void emitPdfReportSelect(const DatePeriod * period);

#endif
