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

program: sentences 						{ $$ = sentencesProgramSemanticAction($1); }
	;

sentences: sentences sentence			{ $$ = sentencesSentenceSemanticAction($1, $2); }
	| sentence							{ $$ = sentenceSemanticAction($1); }
	;

sentence: currencySentence 				{ $$ = sentenceFromCurrencySemanticAction($1); }
	| expenseSentence 					{ $$ = sentenceFromExpenseSemanticAction($1); }
	| incomeSentence 					{ $$ = sentenceFromIncomeSemanticAction($1); }
	| subscriptionSentence 				{ $$ = sentenceFromSubscriptionSemanticAction($1); }
	| querySentence 					{ $$ = sentenceFromQuerySemanticAction($1); }
	| editSentence 						{ $$ = sentenceFromEditSemanticAction($1); }
	| deleteSentence 					{ $$ = sentenceFromDeleteSemanticAction($1); }
	| reportSentence 					{ $$ = sentenceFromReportSemanticAction($1); }
	| finalizeSentence 					{ $$ = sentenceFromFinalizeSemanticAction($1); }
	;

currencySentence: DIVISA ID 			{$$ = currencySentenceSemanticAction($2); }
	;

expenseSentence: GASTO NUMERO optionalInstallments optionalCategory optionalDate optionalDescription {$$ = expenseSentenceSemanticAction($2, $3, $4, $5, $6); }
	;


incomeSentence: INGRESO NUMERO optionalCategory optionalDate optionalDescription {$$ = incomeSentenceSemanticAction($2, $3, $4, $5); }
	;

	
subscriptionSentence: SUSCRIPCION NUMERO frequency optionalCategory optionalFrom optionalUntil optionalDescription { $$ = subscriptionSentenceSemanticAction($2, $3, $4, $5, $6, $7); }
	;

querySentence: CONSULTAR datePeriod { $$ = querySentenceSemanticAction($2); }
	;


editSentence: EDITAR NUMERO editFields { $$ = editSentenceSemanticAction($2, $3); }
	;

deleteSentence: ELIMINAR NUMERO { $$ = deleteSentenceSemanticAction($2); }
	;

reportSentence: REPORTE reportFormat datePeriod { $$ = reportSentenceSemanticAction($2, $3); }
	;

finalizeSentence: FINALIZAR NUMERO { $$ = finalizeSentenceSemanticAction($2); }
	;

optionalInstallments: CUOTAS NUMERO { $$ = presentOptionalInstallmentsSemanticAction($2); }
	| %empty { $$ = emptyOptionalInstallmentsSemanticAction(); }
	;

optionalCategory: CATEGORIA ID { $$ = presentOptionalCategorySemanticAction($2); }
	| %empty { $$ = emptyOptionalCategorySemanticAction(); }
	;

optionalDate: FECHA date { $$ = presentOptionalDateSemanticAction($2); }
	| %empty { $$ = emptyOptionalDateSemanticAction(); }
	;

optionalFrom: DESDE date { $$ = presentOptionalFromSemanticAction($2); }
	| %empty { $$ = emptyOptionalFromSemanticAction(); }
	;

optionalUntil: HASTA date { $$ = presentOptionalUntilSemanticAction($2); }
	| %empty { $$ = emptyOptionalUntilSemanticAction(); }
	;

optionalDescription: DESCRIPCION STRING { $$ = presentOptionalDescriptionSemanticAction($2); }
	| %empty { $$ = emptyOptionalDescriptionSemanticAction(); }
	;

datePeriod: DESDE date HASTA date { $$ = dateRangePeriodSemanticAction($2, $4); }
	| frequency { $$ = frequencyDatePeriodSemanticAction($1); }
	;

frequency: MENSUAL { $$ = frequencySemanticAction(FREQUENCY_MONTHLY); }
	| SEMANAL { $$ = frequencySemanticAction(FREQUENCY_WEEKLY); }
	| ANUAL { $$ = frequencySemanticAction(FREQUENCY_YEARLY); }
	;

editFields: editFields editField { $$ = consEditFieldsSemanticAction($1, $2); }
	| editField { $$ = singleEditFieldSemanticAction($1); }
	;

editField: MONTO NUMERO { $$ = amountEditFieldSemanticAction($2); }
	| CATEGORIA ID { $$ = categoryEditFieldSemanticAction($2); }
	| FECHA date { $$ = dateEditFieldSemanticAction($2); }
	| DESCRIPCION STRING { $$ = descriptionEditFieldSemanticAction($2); }
	;

date: DATE { $$ = stringDateSemanticAction($1); }
	| HOY { $$ = relativeDateSemanticAction(DATE_KIND_TODAY); }
	| AYER { $$ = relativeDateSemanticAction(DATE_KIND_YESTERDAY); }
	| MANIANA { $$ = relativeDateSemanticAction(DATE_KIND_TOMORROW); }
	;

reportFormat: HTML { $$ = reportFormatSemanticAction(REPORT_FORMAT_HTML); }
	| TEXTO_PLANO { $$ = reportFormatSemanticAction(REPORT_FORMAT_PLAIN_TEXT); }
	| PDF { $$ = reportFormatSemanticAction(REPORT_FORMAT_PDF); }
	;

%%
