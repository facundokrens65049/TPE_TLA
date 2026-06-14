#ifndef HTML_REPORT_HEADER
#define HTML_REPORT_HEADER

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

// Emits a SELECT (without trailing ';\n') that returns a SINGLE row holding
// a complete HTML5 document: DOCTYPE + <head> with inline CSS + <body> with
// two <table>s (period data and net balance by currency). The browser
// handles the natural wrapping of cells, so multi-line row expansion is not
// needed. Negative balance is painted red and positive green via CSS classes.
void emitHtmlReportSelect(const DatePeriod * period);

#endif
