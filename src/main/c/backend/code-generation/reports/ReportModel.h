#ifndef REPORT_MODEL_HEADER
#define REPORT_MODEL_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../../support/language/DateUtils.h"
#include <stdbool.h>

#define COL_ID_WIDTH 5
#define COL_TIPO_WIDTH 14
#define COL_MONTO_WIDTH 13
#define COL_DIVISA_WIDTH 7
#define COL_CATEGORIA_WIDTH 18
#define COL_FECHA_WIDTH 10
#define COL_DETALLE_WIDTH 8
#define COL_DESCRIPCION_WIDTH 50

// Extremos del periodo en ISO (YYYY-MM-DD, 10 chars + '\0', asi que los
// buffers deben tener al menos 11 bytes). Un rango usa los extremos ya
// validados; una frecuencia es una ventana que cierra hoy y va un periodo
// para atras (mensual = -1 mes, semanal = -7 dias, anual = -12 meses).
void resolvePeriodBounds(const DatePeriod * period, char * fromBuffer, char * toBuffer);

// Emite el CTE 'cabecera' (compartido por los reportes texto plano y PDF, no
// por el HTML que usa <th>).
void emitReportHeaderCTE(void);

// Emite el subgrafo de CTEs compartido por los reportes (modelo de filas +
// wrap multi-linea + balance).
void emitReportRowsCTE(const char * fromBuffer, const char * toBuffer, bool pdfEscape);

#endif
