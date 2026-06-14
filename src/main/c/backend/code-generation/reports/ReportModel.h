#ifndef REPORT_MODEL_HEADER
#define REPORT_MODEL_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../../support/language/DateUtils.h"
#include <stdbool.h>

#define COL_ID_WIDTH 5
#define COL_TYPE_WIDTH 14
#define COL_AMOUNT_WIDTH 13
#define COL_CURRENCY_WIDTH 7
#define COL_CATEGORY_WIDTH 18
#define COL_DATE_WIDTH 10
#define COL_DETAIL_WIDTH 8
#define COL_DESCRIPTION_WIDTH 50

// Period bounds in ISO format (YYYY-MM-DD, 10 chars + '\0', so the buffers
// must hold at least 11 bytes). A range uses the already-validated bounds;
// a frequency is a window that closes today and goes one period backwards
// (monthly = -1 month, weekly = -7 days, yearly = -12 months).
void resolvePeriodBounds(const DatePeriod * period, char * fromBuffer, char * toBuffer);

// Emits the 'cabecera' CTE (shared by the plain-text and PDF reports, not by
// the HTML one which uses <th>).
void emitReportHeaderCTE(void);

// Emits the CTE subgraph shared by the reports (row model + multi-line wrap
// + balance).
void emitReportRowsCTE(const char * fromBuffer, const char * toBuffer, bool pdfEscape);

#endif
