#include "Frontend.h"

/* MODULE INTERNAL STATE */

static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownFrontendModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Frontend...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFrontendModule(LexicalAnalyzer * lexicalAnalyzer) {
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("Frontend");
	return _shutdownFrontendModule;
}

/* IMPORTED FUNCTIONS */

extern bool flexHasBuffer(LexicalAnalyzer * lexicalAnalyzer);
extern FlexContext flexCurrentContext(LexicalAnalyzer * lexicalAnalyzer);
extern void flexEnterContext(LexicalAnalyzer * lexicalAnalyzer, FlexContext flexContext);
extern void flexLeaveContext(LexicalAnalyzer * lexicalAnalyzer);

/* PRIVATE FUNCTIONS */

static const char * _compilationStatusAsString(const CompilationStatus compilationStatus) {
	switch (compilationStatus) {
		case FAILED:
			return "FAILED";
		case IN_PROGRESS:
			return "IN_PROGRESS";
		case OUT_OF_MEMORY:
			return "OUT_OF_MEMORY";
		case SUCCEEDED:
			return "SUCCEEDED";
		default:
			return "UNKNOWN_ERROR";
	}
}

/* PUBLIC FUNCTIONS */

InputBuffer * createInputBuffer(LexicalAnalyzer * lexicalAnalyzer, const char * path) {
	// Abrimos primero: si el archivo no existe no tiene sentido reservar el
	// wrapper ni armar el buffer de Flex con un FILE * NULL. El caller debe
	// contemplar el retorno NULL.
	FILE * file = fopen(path, "r");
	if (file == NULL) {
		logError(_logger, "Cannot open input file: \"%s\".", path);
		return NULL;
	}
	InputBuffer * inputBuffer = (InputBuffer *) calloc(1, sizeof(InputBuffer));
	inputBuffer->bufferSizeInBytes = YY_BUF_SIZE;
	inputBuffer->file = file;
	inputBuffer->lexicalAnalyzer = lexicalAnalyzer;
	inputBuffer->buffer = yy_create_buffer(file, inputBuffer->bufferSizeInBytes, lexicalAnalyzer->scanner);
	return inputBuffer;
}

LexicalAnalyzer * createLexicalAnalyzer() {
	LexicalAnalyzer * lexicalAnalyzer = (LexicalAnalyzer *) calloc(1, sizeof(LexicalAnalyzer));
	lexicalAnalyzer->location = calloc(1, sizeof(YYLTYPE));
	lexicalAnalyzer->logger = createLogger("LexicalAnalyzer");
	yylex_init(&lexicalAnalyzer->scanner);
	lexicalAnalyzer->parser = yypstate_new();
	flexEnterContext(lexicalAnalyzer, 0);
	return lexicalAnalyzer;
}

Token * createToken(LexicalAnalyzer * lexicalAnalyzer, TokenLabel label) {
	Token * token = (Token *) calloc(1, sizeof(Token));
	token->context = flexCurrentContext(lexicalAnalyzer);
	token->label = label;
	token->length = yyget_leng(lexicalAnalyzer->scanner);
	token->lexeme = (char *) calloc(token->length + 1, sizeof(char));
	token->line = yyget_lineno(lexicalAnalyzer->scanner);
	token->semanticValue = (SemanticValue *) calloc(1, sizeof(SemanticValue));
	strncpy(token->lexeme, yyget_text(lexicalAnalyzer->scanner), token->length);
	return token;
}

FlexContext currentLexicalAnalyzerContext(LexicalAnalyzer * lexicalAnalyzer) {
	return flexCurrentContext(lexicalAnalyzer);
}

void destroyInputBuffer(InputBuffer * inputBuffer) {
	if (inputBuffer != NULL) {
		// El buffer de Flex (creado con yy_create_buffer y empujado con
		// yypush_buffer_state) pertenece a la PILA de Flex, no a este wrapper,
		// asi que NO se borra aca. Ambos caminos lo liberan exactamente una vez:
		//   - EOF normal: popInputBuffer -> yypop_buffer_state ya lo borro.
		//   - error de sintaxis (sin llegar al EOF): queda en la pila y lo borra
		//     yylex_destroy() (en destroyLexicalAnalyzer), que drena la pila
		//     entera. Verificado en FlexScanner.c (Flex 2.6.4).
		// Llamar yy_delete_buffer aca seria un double-free en el primer caso.
		inputBuffer->buffer = NULL;
		if (inputBuffer->file != NULL) {
			fclose(inputBuffer->file);
			inputBuffer->file = NULL;
		}
		inputBuffer->bufferSizeInBytes = 0;
		inputBuffer->lexicalAnalyzer = NULL;
		free(inputBuffer);
	}
}

void destroyLexicalAnalyzer(LexicalAnalyzer * lexicalAnalyzer) {
	if (lexicalAnalyzer != NULL) {
		if (lexicalAnalyzer->parser != NULL) {
			yypstate_delete((yypstate *) lexicalAnalyzer->parser);
			lexicalAnalyzer->parser = NULL;
		}
		if (lexicalAnalyzer->scanner != NULL) {
			yylex_destroy((yyscan_t) lexicalAnalyzer->scanner);
			lexicalAnalyzer->scanner = NULL;
		}
		if (lexicalAnalyzer->logger != NULL) {
			destroyLogger(lexicalAnalyzer->logger);
			lexicalAnalyzer->logger = NULL;
		}
		if (lexicalAnalyzer->location != NULL) {
			free(lexicalAnalyzer->location);
			lexicalAnalyzer->location = NULL;
		}
		free(lexicalAnalyzer);
	}
}

void destroyToken(Token * token) {
	if (token != NULL) {
		if (token->lexeme != NULL) {
			free(token->lexeme);
			token->lexeme = NULL;
		}
		if (token->semanticValue != NULL) {
			free(token->semanticValue);
			token->semanticValue = NULL;
		}
		free(token);
	}
}

void enterLexicalAnalyzerContext(LexicalAnalyzer * lexicalAnalyzer, FlexContext flexContext) {
	flexEnterContext(lexicalAnalyzer, flexContext);
}

CompilationStatus executeLexicalAnalysis(LexicalAnalyzer * lexicalAnalyzer) {
	return (CompilationStatus) yylex(
		NULL,
		(YYLTYPE *) lexicalAnalyzer->location,
		lexicalAnalyzer->scanner);
}

CompilationStatus executeSyntacticAnalysis() {
	logDebugging(_logger, "Parsing...");
	CompilationStatus status = IN_PROGRESS;
	while (status == IN_PROGRESS) {
		status = executeLexicalAnalysis(_lexicalAnalyzer);
	}
	logDebugging(_logger, "Compilation status: %s.", _compilationStatusAsString(status));
	logDebugging(_logger, "Parsing is done.");
	return status;
}

void leaveLexicalAnalyzerContext(LexicalAnalyzer * lexicalAnalyzer) {
	flexLeaveContext(lexicalAnalyzer);
}

bool popInputBuffer(LexicalAnalyzer * lexicalAnalyzer) {
	yypop_buffer_state((yyscan_t) lexicalAnalyzer->scanner);
	return flexHasBuffer(lexicalAnalyzer);
}

void pushInputBuffer(InputBuffer * inputBuffer) {
	yypush_buffer_state((YY_BUFFER_STATE) inputBuffer->buffer, (yyscan_t) inputBuffer->lexicalAnalyzer->scanner);
}

CompilationStatus pushToken(LexicalAnalyzer * lexicalAnalyzer, Token * token) {
	return (CompilationStatus) yypush_parse(
		(yypstate *) lexicalAnalyzer->parser,
		token->label,
		token->semanticValue,
		(YYLTYPE *) lexicalAnalyzer->location);
}
