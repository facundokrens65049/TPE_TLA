#include "EmitSql.h"
#include <stdarg.h>
#include <stdio.h>

void emitSql(const char * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	vfprintf(stdout, format, arguments);
	va_end(arguments);
	fflush(stdout);
}
