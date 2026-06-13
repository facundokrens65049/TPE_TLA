#ifndef TEXT_REPORT_HEADER
#define TEXT_REPORT_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emite un SELECT (sin ';\n' final) que devuelve UNA SOLA fila con el reporte
// formateado como tabla ASCII de ancho fijo: cabecera + separador + filas con
// wrap multi-linea (categoria y descripcion no se truncan, se reparten en
// lineas debajo) + balance neto por divisa al pie. Comparte el modelo de
// filas y la cabecera con el PDF (mismas COL_*_WIDTH).
void emitTextReportSelect(const DatePeriod * period);

#endif
