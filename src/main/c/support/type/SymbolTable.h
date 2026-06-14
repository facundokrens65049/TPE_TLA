#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include <stddef.h>

// Tabla de simbolos. El DSL es plano (sin scopes anidados), asi que es un unico
// contexto global: el set de categorias usadas. No valida nada por si misma,
// solo guarda el contexto que consume el backend.
// La divisa vigente NO vive aca: como puede cambiar a lo largo del programa,
// se resuelve por-sentencia durante la generacion de codigo (la unica fase que
// la necesita), no como un unico valor global.
typedef struct SymbolTable {
	char ** categories;			// categorias normalizadas (lower, sin tildes), sin repetidos; heap
	size_t categoryCount;
	size_t categoryCapacity;
} SymbolTable;

#endif
