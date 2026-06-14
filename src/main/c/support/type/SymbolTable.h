#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include <stddef.h>

// Symbol table. The DSL is flat (no nested scopes), so it is a single global
// context: the set of categories used. It validates nothing on its own, it
// just keeps the context that the backend consumes.
// The active currency does NOT live here: since it can change throughout the
// program, it is resolved per-sentence during code generation (the only
// phase that needs it), not as a single global value.
typedef struct SymbolTable {
	char ** categories;			// normalized categories (lower-case, no accents), no duplicates; heap
	size_t categoryCount;
	size_t categoryCapacity;
} SymbolTable;

#endif
