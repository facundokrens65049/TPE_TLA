#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

#include "SymbolTable.h"

/**
 * The global state of the compiler. Should transport every data structure
 * needed across the different phases of a compilation.
 */
typedef struct {
	/**
	 * The root node of the AST.
	 */
	void * abstractSyntaxTree;

	/**
	 * The semantic symbol table, populated during semantic analysis and
	 * consumed by code generation. NULL until the semantic phase builds it.
	 */
	SymbolTable * symbolTable;

	// TODO: Add more configuration.
	// TODO: Add whatever you need.
	// TODO: ...
} CompilerState;

#endif
