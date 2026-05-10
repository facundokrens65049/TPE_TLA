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

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer);

CompilationStatus dateLiteralLexemeAction();
CompilationStatus descriptionKeywordLexemeAction(FlexContext stringContext);
CompilationStatus eofLexemeAction();
CompilationStatus enterMultilineCommentLexemeAction(FlexContext context);
CompilationStatus identifierLexemeAction();
CompilationStatus ignoredLexemeAction();
CompilationStatus keywordLexemeAction(TokenLabel label);
CompilationStatus leaveMultilineCommentLexemeAction();
CompilationStatus leaveStringContextLexemeAction();
CompilationStatus integerLiteralLexemeAction();
CompilationStatus stringLineLexemeAction();
CompilationStatus unknownLexemeAction();

#endif
