%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

extern CompilationStatus dateLiteralLexemeAction();
extern CompilationStatus descriptionKeywordLexemeAction(FlexContext context);
extern CompilationStatus eofLexemeAction();
extern CompilationStatus enterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus identifierLexemeAction();
extern CompilationStatus ignoredLexemeAction();
extern CompilationStatus keywordLexemeAction(TokenLabel label);
extern CompilationStatus leaveMultilineCommentLexemeAction();
extern CompilationStatus leaveStringContextLexemeAction();
extern CompilationStatus integerLiteralLexemeAction();
extern CompilationStatus stringLineLexemeAction();
extern CompilationStatus unknownLexemeAction();

%}

%option case-insensitive

%x MULTILINE_COMMENT
%x STRING_LINE_CONTEXT

whitespace		[[:space:]]+
digit			[0-9]
letter			[a-zA-Z]

%%

"/*"								{ return enterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
"//"[^\n]*							{ return ignoredLexemeAction(); }
<MULTILINE_COMMENT>"*/"				{ return leaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return ignoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return ignoredLexemeAction(); }

<STRING_LINE_CONTEXT>[^\n]+			{ return stringLineLexemeAction(); }
<STRING_LINE_CONTEXT>"\n"			{ return leaveStringContextLexemeAction(); }

"descripci"(\xc3\xb3|\xc3\x93|"o")"n"				{ return descriptionKeywordLexemeAction(STRING_LINE_CONTEXT); }
"suscripci"(\xc3\xb3|\xc3\x93|"o")"n"				{ return keywordLexemeAction(SUBSCRIPTION); }
"categor"(\xc3\xad|\xc3\x8d|"i")"a"				{ return keywordLexemeAction(CATEGORY); }
("ma"(\xc3\xb1|\xc3\x91)"ana")|"maniana"				{ return keywordLexemeAction(KW_TOMORROW); }
"divisa"							{ return keywordLexemeAction(CURRENCY); }
"gasto"								{ return keywordLexemeAction(EXPENSE); }
"ingreso"							{ return keywordLexemeAction(INCOME); }
"consultar"							{ return keywordLexemeAction(QUERY); }
"editar"							{ return keywordLexemeAction(EDIT); }
"eliminar"							{ return keywordLexemeAction(DELETE); }
"reporte"							{ return keywordLexemeAction(REPORT); }
"finalizar"							{ return keywordLexemeAction(FINALIZE); }
"cuotas"							{ return keywordLexemeAction(INSTALLMENTS); }
"fecha"								{ return keywordLexemeAction(DATE_KEYWORD); }
"monto"								{ return keywordLexemeAction(AMOUNT_KEYWORD); }
"mensual"							{ return keywordLexemeAction(PERIOD_MONTHLY); }
"semanal"							{ return keywordLexemeAction(PERIOD_WEEKLY); }
"anual"								{ return keywordLexemeAction(PERIOD_YEARLY); }
"hoy"								{ return keywordLexemeAction(KW_TODAY); }
"ayer"								{ return keywordLexemeAction(KW_YESTERDAY); }
"desde"								{ return keywordLexemeAction(KW_FROM); }
"hasta"								{ return keywordLexemeAction(KW_UNTIL); }
"html"								{ return keywordLexemeAction(HTML); }
"texto_plano"						{ return keywordLexemeAction(PLAIN_TEXT); }
"texto"[ \t]+"plano"					{ return keywordLexemeAction(PLAIN_TEXT); }
"pdf"								{ return keywordLexemeAction(PDF); }

{digit}{2}"-"{digit}{2}"-"{digit}{4}			{ return dateLiteralLexemeAction(); }

{digit}+							{ return integerLiteralLexemeAction(); }

({letter}|\xc3[\x80-\xbf])({letter}|{digit}|"_"|\xc3[\x80-\xbf])*		{ return identifierLexemeAction(); }

{whitespace}						{ return ignoredLexemeAction(); }
.									{ return unknownLexemeAction(); }

<<EOF>>								{ return eofLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"
