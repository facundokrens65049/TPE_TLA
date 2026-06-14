#ifndef EMIT_SQL_HEADER
#define EMIT_SQL_HEADER

// Punto unico de escritura del backend hacia stdout.
void emitSql(const char * format, ...);

// Fuerza el volcado del buffer de stdout. emitSql ya no hace flush por llamada;
// se invoca una vez al terminar de generar el script.
void flushSql(void);

#endif
