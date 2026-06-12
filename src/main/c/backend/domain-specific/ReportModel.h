#ifndef REPORT_MODEL_HEADER
#define REPORT_MODEL_HEADER

#include <stdbool.h>

// Constantes del dominio: ancho de cada columna del reporte tabular. Se
// comparten entre los formatos texto plano y PDF (HTML usa <th>/<td> y el
// navegador se encarga del wrap).
#define COL_ID_WIDTH 5
#define COL_TIPO_WIDTH 14
#define COL_MONTO_WIDTH 13
#define COL_DIVISA_WIDTH 7
#define COL_CATEGORIA_WIDTH 18
#define COL_FECHA_WIDTH 10
#define COL_DETALLE_WIDTH 8
#define COL_DESCRIPCION_WIDTH 50

// Construye el CTE 'cabecera' (compartido por los reportes texto plano y PDF,
// no por el HTML que usa <th>). Devuelve un fragmento SQL en heap; el caller
// debe liberarlo con free().
char * buildReportHeaderCTE(void);

// Construye el subgrafo de CTEs compartido por los reportes (modelo de filas
// + wrap multi-linea + balance) para el rango [from, to] en formato ISO
// YYYY-MM-DD. Si 'pdfEscape' es true, agrega ademas el CTE filas_alineadas
// con los escapes propios de los string literals PDF. Devuelve un fragmento
// SQL en heap; el caller debe liberarlo con free().
char * buildReportRowsCTE(const char * from, const char * to, bool pdfEscape);

#endif
