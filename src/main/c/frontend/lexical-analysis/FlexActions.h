#ifndef FLEX_ACTIONS_HEADER
#define FLEX_ACTIONS_HEADER

#include "../../support/configuration/Environment.h"
#include "../../support/language/String.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/LexicalAnalyzer.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/Token.h"
#include "../../support/type/TokenLabel.h"
#include "../Frontend.h"

/** Initialize module's internal state. */
ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer);

CompilationStatus DateLexemeAction();
CompilationStatus DescripcionLexemeAction(FlexContext stringContext);
CompilationStatus EOFLexemeAction();
CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
CompilationStatus IdentifierLexemeAction();
CompilationStatus IgnoredLexemeAction();
CompilationStatus KeywordLexemeAction(TokenLabel label);
CompilationStatus LeaveMultilineCommentLexemeAction();
CompilationStatus LeaveStringContextLexemeAction();
CompilationStatus NumeroLexemeAction();
CompilationStatus StringLexemeAction();
CompilationStatus UnknownLexemeAction();

#endif
