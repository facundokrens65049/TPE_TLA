#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */

typedef enum FormatoTipo FormatoTipo;
typedef enum FechaTipo FechaTipo;
typedef enum FrecuenciaTipo FrecuenciaTipo;
typedef enum PeriodoOFechasTipo PeriodoOFechasTipo;
typedef enum CampoEditarTipo CampoEditarTipo;
typedef enum SentenceType SentenceType;

typedef struct Formato Formato;
typedef struct Fecha Fecha;
typedef struct CampoEditar CampoEditar;
typedef struct CamposEditar CamposEditar;
typedef struct Frecuencia Frecuencia;
typedef struct PeriodoOFechas PeriodoOFechas;
typedef struct OptionalDescripcion OptionalDescripcion;
typedef struct OptionalHasta OptionalHasta;
typedef struct OptionalDesde OptionalDesde;
typedef struct OptionalFecha OptionalFecha;
typedef struct OptionalCategoria OptionalCategoria;
typedef struct OptionalCuotas OptionalCuotas;
typedef struct FinalizarSentence FinalizarSentence;
typedef struct ReporteSentence ReporteSentence;
typedef struct EliminarSentence EliminarSentence;
typedef struct EditarSentence EditarSentence;
typedef struct ConsultaSentence ConsultaSentence;
typedef struct SuscripcionSentence SuscripcionSentence;
typedef struct IngresoSentence IngresoSentence;
typedef struct GastoSentence GastoSentence;
typedef struct DivisaSentence DivisaSentence;
typedef struct Sentence Sentence;
typedef struct Sentences Sentences;
typedef struct Program Program;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */

enum FormatoTipo {
    HTML_TIPO,
    TEXTO_PLANO_TIPO
};

enum FechaTipo {
    DATE_TIPO,
    HOY_TIPO,
    AYER_TIPO,
    MANIANA_TIPO
};

enum FrecuenciaTipo {
    MENSUAL_TIPO,
    SEMANAL_TIPO,
    ANUAL_TIPO
};

enum PeriodoOFechasTipo {
    RANGO_TIPO,
    FRECUENCIA_TIPO
};

enum CampoEditarTipo {
    MONTO_CAMPO,
    CATEGORIA_CAMPO,
    FECHA_CAMPO,
    DESCRIPCION_CAMPO
};

enum SentenceType {
    DIVISA_SENTENCE,
    GASTO_SENTENCE,
    INGRESO_SENTENCE,
    SUSCRIPCION_SENTENCE,
    CONSULTAR_SENTENCE,
    EDITAR_SENTENCE,
    ELIMINAR_SENTENCE,
    REPORTE_SENTENCE,
    FINALIZAR_SENTENCE
};

struct Formato {
    FormatoTipo formatoTipo;
};

struct Fecha {
    FechaTipo type;
    union {
        char * date;
    };
};

struct CampoEditar {
    CampoEditarTipo type;
    union {
        int numero;
        char * id;
        Fecha * fecha;
        char * descripcion;
    };
};

struct CamposEditar {
    CampoEditar * campoEditar;
    CamposEditar * next;
};

struct Frecuencia {
    FrecuenciaTipo type;
};


struct PeriodoOFechas {
    PeriodoOFechasTipo type;
    union {
        struct {
            Fecha * desde;
            Fecha * hasta;
        };
        Frecuencia * frecuencia;
    };
};

struct OptionalDescripcion {
    char * descripcion;
};

struct OptionalHasta {
    Fecha * fecha;
};

struct OptionalDesde {
    Fecha * fecha;
};

struct OptionalFecha {
    Fecha * fecha;
};

struct OptionalCategoria {
    char * id;
};

struct OptionalCuotas {
    int numero;
};

struct FinalizarSentence {
    int numero;
};

struct ReporteSentence {
    Formato * formato;
    PeriodoOFechas * periodoOFechas;
};

struct EliminarSentence {
    int numero;
};

struct EditarSentence {
    int numero;
    CamposEditar * camposEditar;
};

struct ConsultaSentence {
    PeriodoOFechas * periodoOFechas;
};

struct SuscripcionSentence {
    int numero;
    Frecuencia * frecuencia;
    OptionalCategoria * optionalCategoria;
    OptionalDesde * optionalDesde;
    OptionalHasta * optionalHasta;
    OptionalDescripcion * optionalDescripcion;
};

struct IngresoSentence {
    int numero;
    OptionalCategoria * optionalCategoria;
    OptionalFecha * optionalFecha;
    OptionalDescripcion * optionalDescripcion;
};

struct GastoSentence {
    int numero;
    OptionalCuotas * optionalCuotas;
    OptionalCategoria * optionalCategoria;
    OptionalFecha * optionalFecha;
    OptionalDescripcion * optionalDescripcion;
};

struct DivisaSentence {
    char * id;
};

struct Sentence {
    SentenceType type;
    union {
        DivisaSentence * divisaSentence;
        GastoSentence * gastoSentence;
        IngresoSentence * ingresoSentence;
        SuscripcionSentence * suscripcionSentence;
        ConsultaSentence * consultaSentence;
        EditarSentence * editarSentence;
        EliminarSentence * eliminarSentence;
        ReporteSentence * reporteSentence;
        FinalizarSentence * finalizarSentence;
    };
};

struct Sentences {
    Sentence * sentence;
    Sentences * next; // lista enlazada
};

struct Program {
    Sentences * sentences;
};

/**
 * Node recursive super-duper-trambolik-destructors.
 */

void destroyFormato(Formato * formato);
void destroyFecha(Fecha * fecha);
void destroyCampoEditar(CampoEditar * campoEditar);
void destroyCamposEditar(CamposEditar * camposEditar);
void destroyFrecuencia(Frecuencia * frecuencia);
void destroyPeriodoOFechas(PeriodoOFechas * periodoOFechas);
void destroyOptionalDescripcion(OptionalDescripcion * optionalDescripcion);
void destroyOptionalHasta(OptionalHasta * optionalHasta);
void destroyOptionalDesde(OptionalDesde * optionalDesde);
void destroyOptionalFecha(OptionalFecha * optionalFecha);
void destroyOptionalCategoria(OptionalCategoria * optionalCategoria);
void destroyOptionalCuotas(OptionalCuotas * optionalCuotas);
void destroyFinalizarSentence(FinalizarSentence * finalizarSentence);
void destroyReporteSentence(ReporteSentence * reporteSentence);
void destroyEliminarSentence(EliminarSentence * eliminarSentence);
void destroyEditarSentence(EditarSentence * editarSentence);
void destroyConsultarSentence(ConsultaSentence * consultaSentence);
void destroySuscripcionSentence(SuscripcionSentence * suscripcionSentence);
void destroyIngresoSentence(IngresoSentence * ingresoSentence);
void destroyGastoSentence(GastoSentence * gastoSentence);
void destroyDivisaSentence(DivisaSentence * divisaSentence);
void destroySentence(Sentence * sentence);
void destroySentences(Sentences * sentences);
void destroyProgram(Program * program);

#endif
