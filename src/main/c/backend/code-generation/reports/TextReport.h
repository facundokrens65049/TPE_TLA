#ifndef TEXT_REPORT_HEADER
#define TEXT_REPORT_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emits a SELECT (without trailing ';\n') that returns a SINGLE row with
// the report formatted as a fixed-width ASCII table: header + separator +
// rows with multi-line wrap (category and description are not truncated,
// they are split into lines below) + net balance per currency at the foot.
// Shares the row model and the header with the PDF (same COL_*_WIDTH).
void emitTextReportSelect(const DatePeriod * period);

#endif
