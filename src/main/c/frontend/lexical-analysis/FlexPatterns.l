%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus DateLexemeAction();
extern CompilationStatus DescriptionLexemeAction(FlexContext context);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus LeaveStringContextLexemeAction();
extern CompilationStatus NumberLexemeAction();
extern CompilationStatus StringLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Case-insensitive matching for all ASCII patterns.
 *
 * @see https://westes.github.io/flex/manual/Scanner-Options.html
 */
%option case-insensitive

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x MULTILINE_COMMENT
%x STRING_LINE_CONTEXT

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
whitespace		[[:space:]]+
digit			[0-9]
letter			[a-zA-Z]

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"/*"								{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
"//"[^\n]*							{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>"*/"				{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return IgnoredLexemeAction(); }

%{
// La descripcion no usa comillas: "descripcion" mete al scanner en
// STRING_LINE_CONTEXT y ahi todo el resto de la linea (hasta el \n) es un solo
// STRING. Ojo: un token escrito despues de la descripcion en la misma linea
// queda comido por el string.
%}
<STRING_LINE_CONTEXT>[^\n]+			{ return StringLexemeAction(); }
<STRING_LINE_CONTEXT>"\n"			{ return LeaveStringContextLexemeAction(); }

"descripci"(\xc3\xb3|\xc3\x93|"o")"n"				{ return DescriptionLexemeAction(STRING_LINE_CONTEXT); }
"suscripci"(\xc3\xb3|\xc3\x93|"o")"n"				{ return KeywordLexemeAction(SUSCRIPCION); }
"categor"(\xc3\xad|\xc3\x8d|"i")"a"				{ return KeywordLexemeAction(CATEGORIA); }
("ma"(\xc3\xb1|\xc3\x91)"ana")|"maniana"				{ return KeywordLexemeAction(MANIANA); }
"divisa"							{ return KeywordLexemeAction(DIVISA); }
"gasto"								{ return KeywordLexemeAction(GASTO); }
"ingreso"							{ return KeywordLexemeAction(INGRESO); }
"consultar"							{ return KeywordLexemeAction(CONSULTAR); }
"editar"							{ return KeywordLexemeAction(EDITAR); }
"eliminar"							{ return KeywordLexemeAction(ELIMINAR); }
"reporte"							{ return KeywordLexemeAction(REPORTE); }
"finalizar"							{ return KeywordLexemeAction(FINALIZAR); }
"cuotas"							{ return KeywordLexemeAction(CUOTAS); }
"fecha"								{ return KeywordLexemeAction(FECHA); }
"monto"								{ return KeywordLexemeAction(MONTO); }
"mensual"							{ return KeywordLexemeAction(MENSUAL); }
"semanal"							{ return KeywordLexemeAction(SEMANAL); }
"anual"								{ return KeywordLexemeAction(ANUAL); }
"hoy"								{ return KeywordLexemeAction(HOY); }
"ayer"								{ return KeywordLexemeAction(AYER); }
"desde"								{ return KeywordLexemeAction(DESDE); }
"hasta"								{ return KeywordLexemeAction(HASTA); }
"html"								{ return KeywordLexemeAction(HTML); }
"texto"({whitespace}|"_")"plano"	{ return KeywordLexemeAction(TEXTO_PLANO); }
"pdf"								{ return KeywordLexemeAction(PDF); }

{digit}{2}"-"{digit}{2}"-"{digit}{4}			{ return DateLexemeAction(); }

{digit}+([.,]{digit}+)?([ \t]*[kKmMbB])+/[^a-zA-Z0-9_]	{ return NumberLexemeAction(); }
{digit}+([.,]{digit}+)?				{ return NumberLexemeAction(); }

({letter}|\xc3[\x80-\xbf])({letter}|{digit}|"_"|"-"|\xc3[\x80-\xbf])*		{ return IdentifierLexemeAction(); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
