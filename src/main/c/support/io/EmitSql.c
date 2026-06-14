#include "EmitSql.h"
#include <stdarg.h>
#include <stdio.h>

// This function is called hundreds of times per program, so we do NOT
// fflush here (it used to flush on every emitted fragment). The stdout
// buffer is drained when the stream is closed in main's normal exit; if an
// explicit flush is needed before that, use flushSql().
void emitSql(const char * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	vfprintf(stdout, format, arguments);
	va_end(arguments);
}

void flushSql(void) {
	fflush(stdout);
}
