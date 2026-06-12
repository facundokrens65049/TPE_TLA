#include "FlexActions.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/* MODULE INTERNAL STATE */

static bool _logIgnoredLexemes = true;
static InputBuffer * _inputBuffer = NULL;
static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownFlexActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: FlexActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	if (_inputBuffer != NULL) {
		destroyInputBuffer(_inputBuffer);
		_inputBuffer = NULL;
	}
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer) {
	_inputBuffer = NULL;
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("FlexActions");
	_logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
	return _shutdownFlexActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logTokenAction(const char * actionName, Token * token);

/**
 * Logs a lexical-analyzer action over a token in DEBUGGING level.
 */

static void _logTokenAction(const char * actionName, Token * token) {
	char * _lexeme = escape(token->lexeme);
	logDebugging(_logger, WARNING_COLOR "%s" DEFAULT_COLOR ": Token(context=%d, label=%d, length=%d, lexeme=%s\"%s\"%s, line=%d, semanticValue=%p)",
		actionName,
		token->context,
		token->label,
		token->length,
		INFORMATION_COLOR, _lexeme, DEFAULT_COLOR,
		token->line,
		token->semanticValue);
	free(_lexeme);
	_lexeme = NULL;
}

/* PUBLIC FUNCTIONS */

CompilationStatus KeywordLexemeAction(TokenLabel label) {
	Token * token = createToken(_lexicalAnalyzer, label);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

// Multiplica "value" por "factor" detectando overflow contra LLONG_MAX. Devuelve
// false si el producto no entra en long long (overflow).
static bool _safeMultiply(long long * value, long long factor) {
	if (*value != 0 && factor > LLONG_MAX / *value) {
		return false;
	}
	*value *= factor;
	return true;
}

// Parsea el lexema de un NUMERO (puede tener parte decimal o multiplicadores
// K/M/B encadenados, pegados o separados por espacios/tabs) y lo devuelve en
// "out" representado como CENTAVOS (valor x 100). Todo el flujo de monto
// trabaja en centavos para evitar floats.
static bool _parseNumberWithMultipliers(const char * lexeme, long long * out) {
	errno = 0;
	char * cursor = NULL;
	long long whole = strtoll(lexeme, &cursor, 10);
	if (errno == ERANGE || whole < 0) {
		return false;
	}

	long long decimalCents = 0;
	if (*cursor == '.' || *cursor == ',') {
		++cursor;
		// Tomamos los dos primeros digitos (más significativos) como centavos y truncamos el resto:
		// ej, "100.555" -> 100.55, "100.999" -> 100.99 (no redondea). La base
		// guarda NUMERIC(15,2), asi que la precision adicional no es
		// representable y la decision es perderla en silencio en vez de
		// rechazar el programa. No se lanza error si hay más de dos decimales.
		int digits = 0;
		while (isdigit((unsigned char) *cursor)) {
			if (digits < 2) {
				decimalCents = decimalCents * 10 + (*cursor - '0');
			}
			++cursor;
			++digits;
		}
		if (digits == 0) {
			return false;
		}
		if (digits == 1) {
			decimalCents *= 10;
		}
	}

	// Centavos = whole * 100 + decimalCents (asegurado < 100).
	long long value = whole;
	if (!_safeMultiply(&value, 100LL)) {
		return false;
	}
	if (value > LLONG_MAX - decimalCents) {
		return false;
	}
	value += decimalCents;

	while (*cursor != '\0') {
		while (*cursor == ' ' || *cursor == '\t') {
			++cursor;
		}
		if (*cursor == '\0') {
			break;
		}
		long long factor = 0;
		switch (*cursor) {
			case 'k': case 'K': factor = 1000LL; break;
			case 'm': case 'M': factor = 1000000LL; break;
			case 'b': case 'B': factor = 1000000000LL; break;
			default:
				return false;
		}
		++cursor;
		if (!_safeMultiply(&value, factor)) {
			return false;
		}
	}
	*out = value;
	return true;
}

CompilationStatus NumberLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, NUMERO);
	long long value = 0;
	if (!_parseNumberWithMultipliers(token->lexeme, &value)) {
		logError(_logger, "Malformed numeric literal (too many decimals, overflow or bad multiplier): \"%s\".", token->lexeme);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
		return FAILED;
	}
	token->semanticValue->integer = value;
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus DescriptionLexemeAction(FlexContext stringContext) {
	Token * token = createToken(_lexicalAnalyzer, DESCRIPCION);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	enterLexicalAnalyzerContext(_lexicalAnalyzer, stringContext);
	return status;
}

CompilationStatus StringLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, STRING);
	char * start = token->lexeme;
	while (*start == ' ' || *start == '\t') start++;
	char * end = start + strlen(start) - 1;
	while (end > start && (*end == ' ' || *end == '\t' || *end == '\r')) end--;
	size_t len = (size_t)(end - start + 1);
	if (len > 80) len = 80;
	token->semanticValue->string = strndup(start, len);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	leaveLexicalAnalyzerContext(_lexicalAnalyzer);
	return status;
}

CompilationStatus DateLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, DATE);
	token->semanticValue->string = strdup(token->lexeme);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus IdentifierLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, ID);
	token->semanticValue->string = strdup(token->lexeme);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus LeaveStringContextLexemeAction() {
	leaveLexicalAnalyzerContext(_lexicalAnalyzer);
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context) {
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, OPEN_COMMENT);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	enterLexicalAnalyzerContext(_lexicalAnalyzer, context);
	return IN_PROGRESS;
}

CompilationStatus LeaveMultilineCommentLexemeAction() {
	leaveLexicalAnalyzerContext(_lexicalAnalyzer);
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, CLOSE_COMMENT);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus EOFLexemeAction() {
	CompilationStatus status = IN_PROGRESS;
	Token * token = createToken(_lexicalAnalyzer, 0);
	_logTokenAction(__FUNCTION__, token);
	if (!popInputBuffer(_lexicalAnalyzer)) {
		status = pushToken(_lexicalAnalyzer, token);
		FlexContext context = currentLexicalAnalyzerContext(_lexicalAnalyzer);
		if (0 < context) {
			logError(_logger, "The final context is not closed (context=%d).", context);
			status = FAILED;
		}
	}
	destroyToken(token);
	return status;
}

CompilationStatus IgnoredLexemeAction() {
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus UnknownLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, UNKNOWN);
	_logTokenAction(__FUNCTION__, token);
	destroyToken(token);
	return FAILED;
}
