#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions.
 */

// Constant * IntegerConstantSemanticAction(const int value);
// Expression * ArithmeticExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, ExpressionType type);
// Expression * FactorExpressionSemanticAction(Factor * factor);
// Factor * ConstantFactorSemanticAction(Constant * constant);
// Factor * ExpressionFactorSemanticAction(Expression * expression);
// Program * ExpressionProgramSemanticAction(Expression * expression);

Formato * FormatoSemanticAction(FormatoTipo formatoTipo);

Fecha * RelativaFechaSemanticAction(FechaTipo tipo);
Fecha * StringFechaSemanticAction(const char * string);

CampoEditar * DescripcionCampoEditarSemanticAction(const char * string);
CampoEditar * FechaCampoEditarSemanticAction(Fecha * fecha);
CampoEditar * CategoriaCampoEditarSemanticAction(const char * id);
CampoEditar * MontoCampoEditarSemanticAction(const int numero);

CamposEditar * CampoEditarSemanticAction(CampoEditar * campoEditar);
CamposEditar * CamposCampoEditarSemanticAction(CamposEditar * camposEditar, CampoEditar * campoEditar);

Frecuencia * FrecuenciaSemanticAction(FrecuenciaTipo frecuenciaTipo);

PeriodoOFechas * RangoPeriodoOFechasSemanticAction(Fecha * fechaDesde, Fecha * fechaHasta);
PeriodoOFechas * FrecuenciaPeriodoOFechasSemanticAction(Frecuencia * frecuencia);

OptionalDescripcion * EmptyOptionalDescripcionSemanticAction();
OptionalDescripcion * PresentOptionalDescripcionSemanticAction(const char * string);
OptionalHasta * EmptyOptionalHastaSemanticAction();
OptionalHasta * PresentOptionalHastaSemanticAction(Fecha * fecha);
OptionalDesde * EmptyOptionalDesdeSemanticAction();
OptionalDesde * PresentOptionalDesdeSemanticAction(Fecha * fecha);
OptionalFecha * EmptyOptionalFechaSemanticAction();
OptionalFecha * PresentOptionalFechaSemanticAction(Fecha * fecha);
OptionalCategoria * EmptyOptionalCategoriaSemanticAction();
OptionalCategoria * PresentOptionalCategoriaSemanticAction(const char * id);
OptionalCuotas * EmptyOptionalCuotasSemanticAction();
OptionalCuotas * PresentOptionalCuotasSemanticAction(const int numero);

FinalizarSentence * FinalizarSentenceSemanticAction(const int numero);
ReporteSentence * ReporteSentenceSemanticAction(Formato * formato, PeriodoOFechas * periodoOFechas);
EliminarSentence * EliminarSentenceSemanticAction(const int numero);
EditarSentence * EditarSentenceSemanticAction(const int numero, CamposEditar * camposEditar);
ConsultaSentence * ConsultaSentenceSemanticAction(PeriodoOFechas * periodoOFechas);
SuscripcionSentence * SuscripcionSentenceSemanticAction(const int numero, Frecuencia * frecuencia, OptionalCategoria * optionalCategoria, OptionalDesde * optionalDesde, OptionalHasta * optionalHasta, OptionalDescripcion * optionalDescripcion);
IngresoSentence * IngresoSentenceSemanticAction(const int numero, OptionalCategoria * optionalCategoria, OptionalFecha * optionalFecha, OptionalDescripcion * optionalDescripcion);
GastoSentence * GastoSentenceSemanticAction(const int numero, OptionalCuotas * optionalCuotas, OptionalCategoria * optionalCategoria, OptionalFecha * optionalFecha, OptionalDescripcion * optionalDescripcion);
DivisaSentence * DivisaSentenceSemanticAction(const char * id);

Sentence * FinalizarSentenceSentenceSemanticAction(FinalizarSentence * finalizarSentence);
Sentence * ReporteSentenceSentenceSemanticAction(ReporteSentence * reporteSentence);
Sentence * EliminarSentenceSentenceSemanticAction(EliminarSentence * eliminarSentence);
Sentence * EditarSentenceSentenceSemanticAction(EditarSentence * editarSentence);
Sentence * ConsultaSentenceSentenceSemanticAction(ConsultaSentence * consultaSentence);
Sentence * SuscripcionSentenceSentenceSemanticAction(SuscripcionSentence * suscripcionSentence);
Sentence * IngresoSentenceSentenceSemanticAction(IngresoSentence * ingresoSentence);
Sentence * GastoSentenceSentenceSemanticAction(GastoSentence * gastoSentence);
Sentence * DivisaSentenceSentenceSemanticAction(DivisaSentence * divisaSentence);

Sentences * SentenceSemanticAction(Sentence * sentence);
Sentences * SentencesSentenceSemanticAction(Sentences * sentences, Sentence * sentence);

Program * SentencesProgramSemanticAction(Sentences * sentences);


#endif
