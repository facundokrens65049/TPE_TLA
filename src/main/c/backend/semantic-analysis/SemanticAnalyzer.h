#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

/**
 * We reuse the types from the AST for convenience, but the backend layers
 * should ideally be separated from the frontend using domain-specific models.
 */
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticAnalyzerModule();

/**
 * Walks the AST of the program currently held in the compiler state and checks
 * the static semantic rules of the language. Returns SUCCEEDED if the program
 * is semantically valid, or FAILED otherwise.
 *
 * NOTE (Stage III, Module 1): this is currently only the traversal scaffolding;
 * no rule is enforced yet, so every syntactically-valid program is accepted.
 * The actual validations land in Module 2.
 */
CompilationStatus executeSemanticAnalysis(CompilerState * compilerState);

#endif
