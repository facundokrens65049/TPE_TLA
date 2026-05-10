#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

struct Program;

/**
 * The global state of the compiler. Should transport every data structure
 * needed across the different phases of a compilation.
 */
typedef struct {
	/** The root node of the AST. */
	struct Program * abstractSyntaxTree;

	/**
	 * The computed value of the entire program (only for the calculator). You
	 * should change or remove this field, or a random child will die, and it
	 * will be your fault.
	 */
	signed int value;

	/* TODO: symbol table, nested scopes, etc. */

} CompilerState;

#endif
