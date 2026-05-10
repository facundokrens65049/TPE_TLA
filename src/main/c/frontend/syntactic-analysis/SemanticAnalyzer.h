#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

#include "../../support/type/CompilationStatus.h"
#include "AbstractSyntaxTree.h"

CompilationStatus validateProgram(Program * program);

#endif
