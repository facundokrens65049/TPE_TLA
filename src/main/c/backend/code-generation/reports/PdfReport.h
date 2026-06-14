#ifndef PDF_REPORT_HEADER
#define PDF_REPORT_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emits a SELECT (without trailing ';\n') that returns a SINGLE row holding
// a PDF 1.4 document (A4 landscape page, base-14 fonts Helvetica-Bold and
// Courier) built entirely in pure SQL -- no extensions or external
// libraries. Pagination is done by LINES (not rows) over the model shared
// with plain text; the balance is appended at the foot of the last page,
// inside the same BT/ET block as the table. Empty reports produce a single
// page with 'Sin operaciones' + 'Sin movimientos'.
void emitPdfReportSelect(const DatePeriod * period);

#endif
