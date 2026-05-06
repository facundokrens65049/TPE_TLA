%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	signed int integer;
	TokenLabel token;
	char * string;

	/** Non-terminals. */

	Program * program;
	Sentences * sentences;
	Sentence * sentence;
	DivisaSentence * divisaSentence;
	GastoSentence * gastoSentence;
	IngresoSentence * ingresoSentence;
	SuscripcionSentence * suscripcionSentence;
	ConsultaSentence * consultaSentence;
	EditarSentence * editarSentence;
	EliminarSentence * eliminarSentence;
	ReporteSentence * reporteSentence;
	FinalizarSentence * finalizarSentence;
	OptionalCuotas * optionalCuotas;
	OptionalCategoria * optionalCategoria;
	OptionalFecha * optionalFecha;
	OptionalDesde * optionalDesde;
	OptionalHasta * optionalHasta;
	OptionalDescripcion * optionalDescripcion;
	PeriodoOFechas * periodoOFechas;
	Frecuencia * frecuencia;
	CamposEditar * camposEditar;
	CampoEditar * campoEditar;
	Fecha * fecha;
	Formato * formato;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
/* Destructors */
%destructor { free($$); } <string>
%destructor { destroySentences($$); } <sentences>
%destructor { destroySentence($$); } <sentence>
%destructor { destroyDivisaSentence($$); } <divisaSentence>
%destructor { destroyGastoSentence($$); } <gastoSentence>
%destructor { destroyIngresoSentence($$); } <ingresoSentence>
%destructor { destroySuscripcionSentence($$); } <suscripcionSentence>
%destructor { destroyConsultaSentence($$); } <consultaSentence>
%destructor { destroyEditarSentence($$); } <editarSentence>
%destructor { destroyEliminarSentence($$); } <eliminarSentence>
%destructor { destroyReporteSentence($$); } <reporteSentence>
%destructor { destroyFinalizarSentence($$); } <finalizarSentence>
%destructor { destroyOptionalCuotas($$); } <optionalCuotas>
%destructor { destroyOptionalCategoria($$); } <optionalCategoria>
%destructor { destroyOptionalFecha($$); } <optionalFecha>
%destructor { destroyOptionalDesde($$); } <optionalDesde>
%destructor { destroyOptionalHasta($$); } <optionalHasta>
%destructor { destroyOptionalDescripcion($$); } <optionalDescripcion>
%destructor { destroyPeriodoOFechas($$); } <periodoOFechas>
%destructor { destroyFrecuencia($$); } <frecuencia>
%destructor { destroyCamposEditar($$); } <camposEditar>
%destructor { destroyCampoEditar($$); } <campoEditar>
%destructor { destroyFecha($$); } <fecha>
%destructor { destroyFormato($$); } <formato>

/* Internal tokens used by FlexActions for logging (never pushed to the parser) */
%token <token> IGNORED
%token <token> UNKNOWN
%token <token> OPEN_COMMENT
%token <token> CLOSE_COMMENT

/* TERMINALS */
%token <integer> NUMERO
%token <string> STRING
%token <string> DATE
%token <string> ID
%token <token> DIVISA
%token <token> GASTO
%token <token> INGRESO
%token <token> SUSCRIPCION
%token <token> CONSULTAR
%token <token> EDITAR
%token <token> ELIMINAR
%token <token> REPORTE
%token <token> FINALIZAR
%token <token> CUOTAS
%token <token> CATEGORIA
%token <token> FECHA
%token <token> DESCRIPCION
%token <token> MONTO
%token <token> MENSUAL
%token <token> SEMANAL
%token <token> ANUAL
%token <token> HOY
%token <token> AYER
%token <token> MANIANA
%token <token> DESDE
%token <token> HASTA
%token <token> HTML
%token <token> TEXTO_PLANO

/* NON-TERMINALS */
%type <program> program
%type <sentences> sentences
%type <sentence> sentence
%type <divisaSentence> divisaSentence
%type <gastoSentence> gastoSentence
%type <ingresoSentence> ingresoSentence
%type <suscripcionSentence> suscripcionSentence
%type <consultaSentence> consultaSentence
%type <editarSentence> editarSentence
%type <eliminarSentence> eliminarSentence
%type <reporteSentence> reporteSentence
%type <finalizarSentence> finalizarSentence
%type <optionalCuotas> optionalCuotas
%type <optionalCategoria> optionalCategoria
%type <optionalFecha> optionalFecha
%type <optionalDesde> optionalDesde
%type <optionalHasta> optionalHasta
%type <optionalDescripcion> optionalDescripcion
%type <periodoOFechas> periodoOFechas
%type <frecuencia> frecuencia
%type <camposEditar> camposEditar
%type <campoEditar> campoEditar
%type <fecha> fecha
%type <formato> formato

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */

%%

program: sentences 						{ $$ = SentencesProgramSemanticAction($1); }
	;

sentences: sentences sentence			{ $$ = SentencesSentenceSemanticAction($1, $2); }
	| sentence							{ $$ = SentenceSemanticAction($1); }
	;

sentence: divisaSentence 				{ $$ = DivisaSentenceSentenceSemanticAction($1); }
	| gastoSentence 					{ $$ = GastoSentenceSentenceSemanticAction($1); }
	| ingresoSentence 					{ $$ = IngresoSentenceSentenceSemanticAction($1); }
	| suscripcionSentence 				{ $$ = SuscripcionSentenceSentenceSemanticAction($1); }
	| consultaSentence 					{ $$ = ConsultaSentenceSentenceSemanticAction($1); }
	| editarSentence 					{ $$ = EditarSentenceSentenceSemanticAction($1); }
	| eliminarSentence 					{ $$ = EliminarSentenceSentenceSemanticAction($1); }
	| reporteSentence 					{ $$ = ReporteSentenceSentenceSemanticAction($1); }
	| finalizarSentence 				{ $$ = FinalizarSentenceSentenceSemanticAction($1); }
	;

divisaSentence: DIVISA ID 				{$$ = DivisaSentenceSemanticAction($2); }
	;

gastoSentence: GASTO NUMERO optionalCuotas optionalCategoria optionalFecha optionalDescripcion {$$ = GastoSentenceSemanticAction($2, $3, $4, $5, $6); }
	;


ingresoSentence: INGRESO NUMERO optionalCategoria optionalFecha optionalDescripcion {$$ = IngresoSentenceSemanticAction($2, $3, $4, $5); }
	;

	
suscripcionSentence: SUSCRIPCION NUMERO frecuencia optionalCategoria optionalDesde optionalHasta optionalDescripcion { $$ = SuscripcionSentenceSemanticAction($2, $3, $4, $5, $6, $7); }
	;

consultaSentence: CONSULTAR periodoOFechas { $$ = ConsultaSentenceSemanticAction($2); }
	;


editarSentence: EDITAR NUMERO camposEditar { $$ = EditarSentenceSemanticAction($2, $3); }
	;

eliminarSentence: ELIMINAR NUMERO { $$ = EliminarSentenceSemanticAction($2); }
	;

reporteSentence: REPORTE formato periodoOFechas { $$ = ReporteSentenceSemanticAction($2, $3); }
	;

finalizarSentence: FINALIZAR NUMERO { $$ = FinalizarSentenceSemanticAction($2); }
	;

optionalCuotas: CUOTAS NUMERO { $$ = PresentOptionalCuotasSemanticAction($2); }
	| %empty { $$ = EmptyOptionalCuotasSemanticAction(); }
	;

optionalCategoria: CATEGORIA ID { $$ = PresentOptionalCategoriaSemanticAction($2); }
	| %empty { $$ = EmptyOptionalCategoriaSemanticAction(); }
	;

optionalFecha: FECHA fecha { $$ = PresentOptionalFechaSemanticAction($2); }
	| %empty { $$ = EmptyOptionalFechaSemanticAction(); }
	;

optionalDesde: DESDE fecha { $$ = PresentOptionalDesdeSemanticAction($2); }
	| %empty { $$ = EmptyOptionalDesdeSemanticAction(); }
	;

optionalHasta: HASTA fecha { $$ = PresentOptionalHastaSemanticAction($2); }
	| %empty { $$ = EmptyOptionalHastaSemanticAction(); }
	;

optionalDescripcion: DESCRIPCION STRING { $$ = PresentOptionalDescripcionSemanticAction($2); }
	| %empty { $$ = EmptyOptionalDescripcionSemanticAction(); }
	;

periodoOFechas: DESDE fecha HASTA fecha { $$ = RangoPeriodoOFechasSemanticAction($2, $4); }
	| frecuencia { $$ = FrecuenciaPeriodoOFechasSemanticAction($1); }
	;

frecuencia: MENSUAL { $$ = FrecuenciaSemanticAction(MENSUAL_TIPO); }
	| SEMANAL { $$ = FrecuenciaSemanticAction(SEMANAL_TIPO); }
	| ANUAL { $$ = FrecuenciaSemanticAction(ANUAL_TIPO); }
	;

camposEditar: camposEditar campoEditar { $$ = CamposCampoEditarSemanticAction($1, $2); }
	| campoEditar { $$ = CampoEditarSemanticAction($1); }
	;

campoEditar: MONTO NUMERO { $$ = MontoCampoEditarSemanticAction($2); }
	| CATEGORIA ID { $$ = CategoriaCampoEditarSemanticAction($2); }
	| FECHA fecha { $$ = FechaCampoEditarSemanticAction($2); }
	| DESCRIPCION STRING { $$ = DescripcionCampoEditarSemanticAction($2); }
	;

fecha: DATE { $$ = StringFechaSemanticAction($1); }
	| HOY { $$ = RelativaFechaSemanticAction(HOY_TIPO); }
	| AYER { $$ = RelativaFechaSemanticAction(AYER_TIPO); }
	| MANIANA { $$ = RelativaFechaSemanticAction(MANIANA_TIPO); }
	;

formato: HTML { $$ = FormatoSemanticAction(HTML_TIPO); }
	| TEXTO_PLANO { $$ = FormatoSemanticAction(TEXTO_PLANO_TIPO); }
	;

%%