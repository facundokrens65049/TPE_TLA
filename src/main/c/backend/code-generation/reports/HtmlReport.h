#ifndef HTML_REPORT_HEADER
#define HTML_REPORT_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emite un SELECT (sin ';\n' final) que devuelve UNA SOLA fila con un
// documento HTML5 completo: DOCTYPE + <head> con CSS inline + <body> con dos
// <table> (datos del periodo y balance neto por divisa). El navegador se
// encarga del wrap natural de las celdas, asi que no es necesaria la
// expansion multi-linea de filas. El balance negativo se pinta rojo y el
// positivo verde via clases CSS.
void emitHtmlReportSelect(const DatePeriod * period);

#endif
