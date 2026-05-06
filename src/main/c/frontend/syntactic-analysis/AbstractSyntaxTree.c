#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyFormato(Formato * formato){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (formato != NULL) {
		free(formato);
	}
}

void destroyFecha(Fecha * fecha){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fecha != NULL) {
        if (fecha->type == DATE_TIPO) {
            free(fecha->date);
        }
        free(fecha);
    }
}

void destroyCampoEditar(CampoEditar * campoEditar){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (campoEditar != NULL) {
		switch (campoEditar->type) {
			case MONTO_CAMPO:
                break;
            case CATEGORIA_CAMPO:
                free(campoEditar->id);
                break;
            case FECHA_CAMPO:
                destroyFecha(campoEditar->fecha);
                break;
            case DESCRIPCION_CAMPO:
                free(campoEditar->descripcion);
                break;
		}
		free(campoEditar);
	}
}

void destroyCamposEditar(CamposEditar * camposEditar){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (camposEditar != NULL) {
        destroyCampoEditar(camposEditar->campoEditar);
        destroyCamposEditar(camposEditar->next);
        free(camposEditar);
    }
}

void destroyFrecuencia(Frecuencia * frecuencia){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (frecuencia != NULL) {
		free(frecuencia);
	}
}

void destroyPeriodoOFechas(PeriodoOFechas * periodoOFechas){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (periodoOFechas != NULL) {
		switch (periodoOFechas->type) {
            case RANGO_TIPO:
                destroyFecha(periodoOFechas->desde);
                destroyFecha(periodoOFechas->hasta);
                break;
            case FRECUENCIA_TIPO:
                destroyFrecuencia(periodoOFechas->frecuencia);
                break;
        }
        free(periodoOFechas);
	}
}

void destroyOptionalDescripcion(OptionalDescripcion * optionalDescripcion){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if(optionalDescripcion != NULL) {
		free(optionalDescripcion->descripcion);
		free(optionalDescripcion);
	}
}

void destroyOptionalHasta(OptionalHasta * optionalHasta){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optionalHasta != NULL) {
		destroyFecha(optionalHasta->fecha);
		free(optionalHasta);
	}
}

void destroyOptionalDesde(OptionalDesde * optionalDesde){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optionalDesde != NULL) {
		destroyFecha(optionalDesde->fecha);
		free(optionalDesde);
	}
}

void destroyOptionalFecha(OptionalFecha * optionalFecha){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optionalFecha != NULL) {
		destroyFecha(optionalFecha->fecha);
		free(optionalFecha);
	}
}

void destroyOptionalCategoria(OptionalCategoria * optionalCategoria){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optionalCategoria != NULL) {
		free(optionalCategoria->id);
		free(optionalCategoria);
	}
}

void destroyOptionalCuotas(OptionalCuotas * optionalCuotas){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (optionalCuotas != NULL) {
		free(optionalCuotas);
	}
}

void destroyFinalizarSentence(FinalizarSentence * finalizarSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (finalizarSentence != NULL) {
		free(finalizarSentence);
	}
}

void destroyReporteSentence(ReporteSentence * reporteSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (reporteSentence != NULL) {
		destroyFormato(reporteSentence->formato);
		destroyPeriodoOFechas(reporteSentence->periodoOFechas);
		free(reporteSentence);
	}
}

void destroyEliminarSentence(EliminarSentence * eliminarSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (eliminarSentence != NULL) {
		free(eliminarSentence);
	}
}

void destroyEditarSentence(EditarSentence * editarSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (editarSentence != NULL) {
		destroyCamposEditar(editarSentence->camposEditar);
		free(editarSentence);
	}
}

void destroyConsultaSentence(ConsultaSentence * consultaSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (consultaSentence != NULL) {
		destroyPeriodoOFechas(consultaSentence->periodoOFechas);
		free(consultaSentence);
	}
}

void destroySuscripcionSentence(SuscripcionSentence * suscripcionSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (suscripcionSentence != NULL) {
		destroyFrecuencia(suscripcionSentence->frecuencia);
        destroyOptionalCategoria(suscripcionSentence->optionalCategoria);
        destroyOptionalDesde(suscripcionSentence->optionalDesde);
        destroyOptionalHasta(suscripcionSentence->optionalHasta);
        destroyOptionalDescripcion(suscripcionSentence->optionalDescripcion);
        free(suscripcionSentence);
	}
}
void destroyIngresoSentence(IngresoSentence * ingresoSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (ingresoSentence != NULL) {
		destroyOptionalCategoria(ingresoSentence->optionalCategoria);
        destroyOptionalFecha(ingresoSentence->optionalFecha);
        destroyOptionalDescripcion(ingresoSentence->optionalDescripcion);
        free(ingresoSentence);
	}
}
void destroyGastoSentence(GastoSentence * gastoSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (gastoSentence != NULL) {
		destroyOptionalCuotas(gastoSentence->optionalCuotas);
        destroyOptionalCategoria(gastoSentence->optionalCategoria);
        destroyOptionalFecha(gastoSentence->optionalFecha);
        destroyOptionalDescripcion(gastoSentence->optionalDescripcion);
        free(gastoSentence);
	}
}
void destroyDivisaSentence(DivisaSentence * divisaSentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (divisaSentence != NULL) {
		free(divisaSentence->id);
		free(divisaSentence);
	}
}

void destroySentence(Sentence * sentence){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentence != NULL) {
        switch (sentence->type) {
            case DIVISA_SENTENCE:
                destroyDivisaSentence(sentence->divisaSentence);
                break;
            case GASTO_SENTENCE:
                destroyGastoSentence(sentence->gastoSentence);
                break;
            case INGRESO_SENTENCE:
                destroyIngresoSentence(sentence->ingresoSentence);
                break;
            case SUSCRIPCION_SENTENCE:
                destroySuscripcionSentence(sentence->suscripcionSentence);
                break;
            case CONSULTAR_SENTENCE:
                destroyConsultaSentence(sentence->consultaSentence);
                break;
            case EDITAR_SENTENCE:
                destroyEditarSentence(sentence->editarSentence);
                break;
            case ELIMINAR_SENTENCE:
                destroyEliminarSentence(sentence->eliminarSentence);
                break;
            case REPORTE_SENTENCE:
                destroyReporteSentence(sentence->reporteSentence);
                break;
            case FINALIZAR_SENTENCE:
                destroyFinalizarSentence(sentence->finalizarSentence);
                break;
        }
        free(sentence);
    }
}

void destroySentences(Sentences * sentences){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sentences != NULL) {
        destroySentence(sentences->sentence);
        destroySentences(sentences->next);
        free(sentences);
    }
}

void destroyProgram(Program * program){
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
        destroySentences(program->sentences);
        free(program);
    }
}