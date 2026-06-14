#include "EmitSql.h"
#include <stdarg.h>
#include <stdio.h>

// Esta funcion se llama cientos de veces por programa, asi que NO se hace
// fflush aca (era un flush por cada fragmento emitido). El buffer de stdout se
// vacia solo al cerrar el stream en la salida normal de main; si se necesita un
// volcado explicito antes de eso, usar flushSql().
void emitSql(const char * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	vfprintf(stdout, format, arguments);
	va_end(arguments);
}

void flushSql(void) {
	fflush(stdout);
}
