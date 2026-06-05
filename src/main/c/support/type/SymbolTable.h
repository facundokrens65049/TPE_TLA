#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include <stddef.h>

// Tabla de simbolos. El DSL es plano (sin scopes anidados), asi que es un unico
// contexto global: la divisa activa y el set de categorias usadas. No valida
// nada por si misma, solo guarda el contexto que consume el backend.
typedef struct SymbolTable {
	char * activeCurrency;		// ultima "divisa" declarada o la default; heap
	char ** categories;			// categorias normalizadas (lower, sin tildes), sin repetidos; heap
	size_t categoryCount;
	size_t categoryCapacity;
} SymbolTable;

#endif
