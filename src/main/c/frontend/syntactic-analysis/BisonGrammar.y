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

	long long integer;
	TokenLabel token;
	char * string;

	/** Non-terminals. */

	Program * program;
	Sentences * sentences;
	Sentence * sentence;
	CurrencySentence * currencySentence;
	ExpenseSentence * expenseSentence;
	IncomeSentence * incomeSentence;
	SubscriptionSentence * subscriptionSentence;
	QuerySentence * querySentence;
	EditSentence * editSentence;
	DeleteSentence * deleteSentence;
	ReportSentence * reportSentence;
	FinalizeSentence * finalizeSentence;
	OptionalInstallments * optionalInstallments;
	OptionalCategory * optionalCategory;
	OptionalDate * optionalDate;
	OptionalFrom * optionalFrom;
	OptionalUntil * optionalUntil;
	OptionalDescription * optionalDescription;
	DatePeriod * datePeriod;
	Frequency * frequency;
	EditFieldList * editFields;
	EditField * editField;
	Date * astDate;
	ReportFormat * reportFormat;
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
%destructor { destroyCurrencySentence($$); } <currencySentence>
%destructor { destroyExpenseSentence($$); } <expenseSentence>
%destructor { destroyIncomeSentence($$); } <incomeSentence>
%destructor { destroySubscriptionSentence($$); } <subscriptionSentence>
%destructor { destroyQuerySentence($$); } <querySentence>
%destructor { destroyEditSentence($$); } <editSentence>
%destructor { destroyDeleteSentence($$); } <deleteSentence>
%destructor { destroyReportSentence($$); } <reportSentence>
%destructor { destroyFinalizeSentence($$); } <finalizeSentence>
%destructor { destroyOptionalInstallments($$); } <optionalInstallments>
%destructor { destroyOptionalCategory($$); } <optionalCategory>
%destructor { destroyOptionalDate($$); } <optionalDate>
%destructor { destroyOptionalFrom($$); } <optionalFrom>
%destructor { destroyOptionalUntil($$); } <optionalUntil>
%destructor { destroyOptionalDescription($$); } <optionalDescription>
%destructor { destroyDatePeriod($$); } <datePeriod>
%destructor { destroyFrequency($$); } <frequency>
%destructor { destroyEditFieldList($$); } <editFields>
%destructor { destroyEditField($$); } <editField>
%destructor { destroyDate($$); } <astDate>
%destructor { destroyReportFormat($$); } <reportFormat>

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
%token <token> PDF

/* NON-TERMINALS */
%type <program> program
%type <sentences> sentences
%type <sentence> sentence
%type <currencySentence> currencySentence
%type <expenseSentence> expenseSentence
%type <incomeSentence> incomeSentence
%type <subscriptionSentence> subscriptionSentence
%type <querySentence> querySentence
%type <editSentence> editSentence
%type <deleteSentence> deleteSentence
%type <reportSentence> reportSentence
%type <finalizeSentence> finalizeSentence
%type <optionalInstallments> optionalInstallments
%type <optionalCategory> optionalCategory
%type <optionalDate> optionalDate
%type <optionalFrom> optionalFrom
%type <optionalUntil> optionalUntil
%type <optionalDescription> optionalDescription
%type <datePeriod> datePeriod
%type <frequency> frequency
%type <editFields> editFields
%type <editField> editField
%type <astDate> date
%type <reportFormat> reportFormat

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

sentence: currencySentence 				{ $$ = SentenceFromCurrencySemanticAction($1); }
	| expenseSentence 					{ $$ = SentenceFromExpenseSemanticAction($1); }
	| incomeSentence 					{ $$ = SentenceFromIncomeSemanticAction($1); }
	| subscriptionSentence 				{ $$ = SentenceFromSubscriptionSemanticAction($1); }
	| querySentence 					{ $$ = SentenceFromQuerySemanticAction($1); }
	| editSentence 						{ $$ = SentenceFromEditSemanticAction($1); }
	| deleteSentence 					{ $$ = SentenceFromDeleteSemanticAction($1); }
	| reportSentence 					{ $$ = SentenceFromReportSemanticAction($1); }
	| finalizeSentence 					{ $$ = SentenceFromFinalizeSemanticAction($1); }
	;

currencySentence: DIVISA ID 			{$$ = CurrencySentenceSemanticAction($2); }
	;

expenseSentence: GASTO NUMERO optionalInstallments optionalCategory optionalDate optionalDescription {$$ = ExpenseSentenceSemanticAction($2, $3, $4, $5, $6); }
	;


incomeSentence: INGRESO NUMERO optionalCategory optionalDate optionalDescription {$$ = IncomeSentenceSemanticAction($2, $3, $4, $5); }
	;

	
subscriptionSentence: SUSCRIPCION NUMERO frequency optionalCategory optionalFrom optionalUntil optionalDescription { $$ = SubscriptionSentenceSemanticAction($2, $3, $4, $5, $6, $7); }
	;

querySentence: CONSULTAR datePeriod { $$ = QuerySentenceSemanticAction($2); }
	;


editSentence: EDITAR NUMERO editFields { $$ = EditSentenceSemanticAction($2, $3); }
	;

deleteSentence: ELIMINAR NUMERO { $$ = DeleteSentenceSemanticAction($2); }
	;

reportSentence: REPORTE reportFormat datePeriod { $$ = ReportSentenceSemanticAction($2, $3); }
	;

finalizeSentence: FINALIZAR NUMERO { $$ = FinalizeSentenceSemanticAction($2); }
	;

optionalInstallments: CUOTAS NUMERO { $$ = PresentOptionalInstallmentsSemanticAction($2); }
	| %empty { $$ = EmptyOptionalInstallmentsSemanticAction(); }
	;

optionalCategory: CATEGORIA ID { $$ = PresentOptionalCategorySemanticAction($2); }
	| %empty { $$ = EmptyOptionalCategorySemanticAction(); }
	;

optionalDate: FECHA date { $$ = PresentOptionalDateSemanticAction($2); }
	| %empty { $$ = EmptyOptionalDateSemanticAction(); }
	;

optionalFrom: DESDE date { $$ = PresentOptionalFromSemanticAction($2); }
	| %empty { $$ = EmptyOptionalFromSemanticAction(); }
	;

optionalUntil: HASTA date { $$ = PresentOptionalUntilSemanticAction($2); }
	| %empty { $$ = EmptyOptionalUntilSemanticAction(); }
	;

optionalDescription: DESCRIPCION STRING { $$ = PresentOptionalDescriptionSemanticAction($2); }
	| %empty { $$ = EmptyOptionalDescriptionSemanticAction(); }
	;

datePeriod: DESDE date HASTA date { $$ = DateRangePeriodSemanticAction($2, $4); }
	| frequency { $$ = FrequencyDatePeriodSemanticAction($1); }
	;

frequency: MENSUAL { $$ = FrequencySemanticAction(FREQUENCY_MONTHLY); }
	| SEMANAL { $$ = FrequencySemanticAction(FREQUENCY_WEEKLY); }
	| ANUAL { $$ = FrequencySemanticAction(FREQUENCY_YEARLY); }
	;

editFields: editFields editField { $$ = ConstructEditFieldsSemanticAction($1, $2); }
	| editField { $$ = SingleEditFieldSemanticAction($1); }
	;

editField: MONTO NUMERO { $$ = AmountEditFieldSemanticAction($2); }
	| CATEGORIA ID { $$ = CategoryEditFieldSemanticAction($2); }
	| FECHA date { $$ = DateEditFieldSemanticAction($2); }
	| DESCRIPCION STRING { $$ = DescriptionEditFieldSemanticAction($2); }
	;

date: DATE { $$ = StringDateSemanticAction($1); }
	| HOY { $$ = RelativeDateSemanticAction(DATE_KIND_TODAY); }
	| AYER { $$ = RelativeDateSemanticAction(DATE_KIND_YESTERDAY); }
	| MANIANA { $$ = RelativeDateSemanticAction(DATE_KIND_TOMORROW); }
	;

reportFormat: HTML { $$ = ReportFormatSemanticAction(REPORT_FORMAT_HTML); }
	| TEXTO_PLANO { $$ = ReportFormatSemanticAction(REPORT_FORMAT_PLAIN_TEXT); }
	| PDF { $$ = ReportFormatSemanticAction(REPORT_FORMAT_PDF); }
	;

%%
