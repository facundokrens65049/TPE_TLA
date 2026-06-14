#ifndef EMIT_SQL_HEADER
#define EMIT_SQL_HEADER

// Single write point from the backend to stdout.
void emitSql(const char * format, ...);

// Forces the stdout buffer to flush. emitSql no longer flushes per call; this
// is invoked once when the script generation finishes.
void flushSql(void);

#endif
