#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

Formato * FormatoSemanticAction(FormatoTipo formatoTipo){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Formato * formato = calloc(1, sizeof(Formato));
	formato->formatoTipo = formatoTipo;
	return formato;
}

Fecha * RelativaFechaSemanticAction(FechaTipo tipo){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Fecha * fecha = calloc(1, sizeof(Fecha));
	fecha->type = tipo;
	return fecha;
}

Fecha * StringFechaSemanticAction(const char * string){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Fecha * fecha = calloc(1, sizeof(Fecha));
	fecha->type = DATE_TIPO;
	fecha->date = strdup(string);
	return fecha;
}

CampoEditar * DescripcionCampoEditarSemanticAction(const char * string){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CampoEditar * campoEditar = calloc(1, sizeof(CampoEditar));
	campoEditar->type = DESCRIPCION_CAMPO;
	campoEditar->descripcion = strdup(string);
	return campoEditar;
}

CampoEditar * FechaCampoEditarSemanticAction(Fecha * fecha){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CampoEditar * campoEditar = calloc(1, sizeof(CampoEditar));
	campoEditar->type = FECHA_CAMPO;
	campoEditar->fecha = fecha;
	return campoEditar;
}

CampoEditar * CategoriaCampoEditarSemanticAction(const char * id){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CampoEditar * campoEditar = calloc(1, sizeof(CampoEditar));
	campoEditar->type = CATEGORIA_CAMPO;
	campoEditar->id = strdup(id);
	return campoEditar;
}

CampoEditar * MontoCampoEditarSemanticAction(const int numero){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CampoEditar * campoEditar = calloc(1, sizeof(CampoEditar));
	campoEditar->type = MONTO_CAMPO;
	campoEditar->numero = numero;
	return campoEditar;
}

CamposEditar * CampoEditarSemanticAction(CampoEditar * campoEditar){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CamposEditar * camposEditar = calloc(1, sizeof(CamposEditar));
	camposEditar->campoEditar = campoEditar;
	return camposEditar;
}

CamposEditar * CamposCampoEditarSemanticAction(CamposEditar * camposEditar, CampoEditar * campoEditar){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    CamposEditar * newCamposEditar = calloc(1, sizeof(CamposEditar));
    newCamposEditar->campoEditar = campoEditar;
    newCamposEditar->next = camposEditar;
    return newCamposEditar;
}

Frecuencia * FrecuenciaSemanticAction(FrecuenciaTipo frecuenciaTipo){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Frecuencia * frecuencia = calloc(1, sizeof(Frecuencia));
	frecuencia->type = frecuenciaTipo;
	return frecuencia;
}

PeriodoOFechas * RangoPeriodoOFechasSemanticAction(Fecha * fechaDesde, Fecha * fechaHasta){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PeriodoOFechas * periodoOFechas = calloc(1, sizeof(PeriodoOFechas));
	periodoOFechas->type = RANGO_TIPO;
	periodoOFechas->desde = fechaDesde;
	periodoOFechas->hasta = fechaHasta;
	return periodoOFechas;
}

PeriodoOFechas * FrecuenciaPeriodoOFechasSemanticAction(Frecuencia * frecuencia){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PeriodoOFechas * periodoOFechas = calloc(1, sizeof(PeriodoOFechas));
	periodoOFechas->type = FRECUENCIA_TIPO;
	periodoOFechas->frecuencia = frecuencia;
	return periodoOFechas;
}

OptionalDescripcion * EmptyOptionalDescripcionSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalDescripcion * PresentOptionalDescripcionSemanticAction(const char * string){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalDescripcion * optionalDescripcion = calloc(1, sizeof(OptionalDescripcion));
	optionalDescripcion->descripcion = strdup(string);
	return optionalDescripcion;
}

OptionalHasta * EmptyOptionalHastaSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalHasta * PresentOptionalHastaSemanticAction(Fecha * fecha){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalHasta * optionalHasta = calloc(1, sizeof(OptionalHasta));
	optionalHasta->fecha = fecha;
	return optionalHasta;
}

OptionalDesde * EmptyOptionalDesdeSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalDesde * PresentOptionalDesdeSemanticAction(Fecha * fecha){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalDesde * optionalDesde = calloc(1, sizeof(OptionalDesde));
	optionalDesde->fecha = fecha;
	return optionalDesde;
}

OptionalFecha * EmptyOptionalFechaSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalFecha * PresentOptionalFechaSemanticAction(Fecha * fecha){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalFecha * optionalFecha = calloc(1, sizeof(OptionalFecha));
	optionalFecha->fecha = fecha;
	return optionalFecha;
}

OptionalCategoria * EmptyOptionalCategoriaSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalCategoria * PresentOptionalCategoriaSemanticAction(const char * id){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalCategoria * optionalCategoria = calloc(1, sizeof(OptionalCategoria));
	optionalCategoria->id = strdup(id);
	return optionalCategoria;
}

OptionalCuotas * EmptyOptionalCuotasSemanticAction(){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

OptionalCuotas * PresentOptionalCuotasSemanticAction(const int numero){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	OptionalCuotas * optionalCuotas = calloc(1, sizeof(OptionalCuotas));
	optionalCuotas->numero = numero;
	return optionalCuotas;
}

FinalizarSentence * FinalizarSentenceSemanticAction(const int numero){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	FinalizarSentence * finalizarSentence = calloc(1, sizeof(FinalizarSentence));
	finalizarSentence->numero = numero;
	return finalizarSentence;
}

ReporteSentence * ReporteSentenceSemanticAction(Formato * formato, PeriodoOFechas * periodoOFechas){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ReporteSentence * reporteSentence = calloc(1, sizeof(ReporteSentence));
	reporteSentence->formato = formato;
	reporteSentence->periodoOFechas = periodoOFechas;
	return reporteSentence;
}

EliminarSentence * EliminarSentenceSemanticAction(const int numero){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EliminarSentence * eliminarSentence = calloc(1, sizeof(EliminarSentence));
	eliminarSentence->numero = numero;
	return eliminarSentence;
}

EditarSentence * EditarSentenceSemanticAction(const int numero, CamposEditar * camposEditar){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EditarSentence * editarSentence = calloc(1, sizeof(EditarSentence));
	editarSentence->numero = numero;
	editarSentence->camposEditar = camposEditar;
	return editarSentence;
}

ConsultaSentence * ConsultaSentenceSemanticAction(PeriodoOFechas * periodoOFechas){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ConsultaSentence * consultaSentence = calloc(1, sizeof(ConsultaSentence));
	consultaSentence->periodoOFechas = periodoOFechas;
	return consultaSentence;
}

SuscripcionSentence * SuscripcionSentenceSemanticAction(const int numero, Frecuencia * frecuencia, OptionalCategoria * optionalCategoria, OptionalDesde * optionalDesde, OptionalHasta * optionalHasta, OptionalDescripcion * optionalDescripcion){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    SuscripcionSentence * suscripcionSentence = calloc(1, sizeof(SuscripcionSentence));
    suscripcionSentence->numero = numero;
    suscripcionSentence->frecuencia = frecuencia;
    suscripcionSentence->optionalCategoria = optionalCategoria;
    suscripcionSentence->optionalDesde = optionalDesde;
    suscripcionSentence->optionalHasta = optionalHasta;
    suscripcionSentence->optionalDescripcion = optionalDescripcion;
    return suscripcionSentence;
}

IngresoSentence * IngresoSentenceSemanticAction(const int numero, OptionalCategoria * optionalCategoria, OptionalFecha * optionalFecha, OptionalDescripcion * optionalDescripcion){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    IngresoSentence * ingresoSentence = calloc(1, sizeof(IngresoSentence));
    ingresoSentence->numero = numero;
    ingresoSentence->optionalCategoria = optionalCategoria;
    ingresoSentence->optionalFecha = optionalFecha;
    ingresoSentence->optionalDescripcion = optionalDescripcion;
    return ingresoSentence;
}

GastoSentence * GastoSentenceSemanticAction(const int numero, OptionalCuotas * optionalCuotas, OptionalCategoria * optionalCategoria, OptionalFecha * optionalFecha, OptionalDescripcion * optionalDescripcion){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    GastoSentence * gastoSentence = calloc(1, sizeof(GastoSentence));
    gastoSentence->numero = numero;
    gastoSentence->optionalCuotas = optionalCuotas;
    gastoSentence->optionalCategoria = optionalCategoria;
    gastoSentence->optionalFecha = optionalFecha;
    gastoSentence->optionalDescripcion = optionalDescripcion;
    return gastoSentence;
}

DivisaSentence * DivisaSentenceSemanticAction(const char * id){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    DivisaSentence * divisaSentence = calloc(1, sizeof(DivisaSentence));
    divisaSentence->id = strdup(id);
    return divisaSentence;
}

Sentence * FinalizarSentenceSentenceSemanticAction(FinalizarSentence * finalizarSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    Sentence * sentence = calloc(1, sizeof(Sentence));
    sentence->type = FINALIZAR_SENTENCE;
    sentence->finalizarSentence = finalizarSentence;
    return sentence;
}

Sentence * ReporteSentenceSentenceSemanticAction(ReporteSentence * reporteSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = REPORTE_SENTENCE;
	sentence->reporteSentence = reporteSentence;
	return sentence;
}

Sentence * EliminarSentenceSentenceSemanticAction(EliminarSentence * eliminarSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = ELIMINAR_SENTENCE;
	sentence->eliminarSentence = eliminarSentence;
	return sentence;
}

Sentence * EditarSentenceSentenceSemanticAction(EditarSentence * editarSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = EDITAR_SENTENCE;
	sentence->editarSentence = editarSentence;
	return sentence;
}

Sentence * ConsultaSentenceSentenceSemanticAction(ConsultaSentence * consultaSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = CONSULTAR_SENTENCE;
	sentence->consultaSentence = consultaSentence;
	return sentence;
}

Sentence * SuscripcionSentenceSentenceSemanticAction(SuscripcionSentence * suscripcionSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = SUSCRIPCION_SENTENCE;
	sentence->suscripcionSentence = suscripcionSentence;
	return sentence;
}

Sentence * IngresoSentenceSentenceSemanticAction(IngresoSentence * ingresoSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = INGRESO_SENTENCE;
	sentence->ingresoSentence = ingresoSentence;
	return sentence;
}

Sentence * GastoSentenceSentenceSemanticAction(GastoSentence * gastoSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = GASTO_SENTENCE;
	sentence->gastoSentence = gastoSentence;
	return sentence;
}

Sentence * DivisaSentenceSentenceSemanticAction(DivisaSentence * divisaSentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Sentence * sentence = calloc(1, sizeof(Sentence));
	sentence->type = DIVISA_SENTENCE;
	sentence->divisaSentence = divisaSentence;
	return sentence;
}

Sentences * SentenceSemanticAction(Sentence * sentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    Sentences * sentences = calloc(1, sizeof(Sentences));
    sentences->sentence = sentence;
    sentences->next = NULL;
    return sentences;
}

Sentences * SentencesSentenceSemanticAction(Sentences * sentences, Sentence * sentence){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    Sentences * newSentences = calloc(1, sizeof(Sentences));
    newSentences->sentence = sentence;
    newSentences->next = sentences;
    return newSentences;
}

Program * SentencesProgramSemanticAction(Sentences * sentences){
	_logSyntacticAnalyzerAction(__FUNCTION__);
    Program * program = calloc(1, sizeof(Program));
    program->sentences = sentences;
    _compilerState->abstractSyntaxtTree = program;
    return program;
}