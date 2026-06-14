#include "String.h"

/* PRIVATE FUNCTIONS */

static const char * _controlCharacterToEscapedString(const char character);

/**
 * Returns a read-only string that represents the escaped sequence of the
 * control character. If the character is not a control, then returns NULL.
 */
static const char * _controlCharacterToEscapedString(const char character) {
	switch (character) {
		case 0x00: return "\\0";
		case 0x01: return "\\x01";
		case 0x02: return "\\x02";
		case 0x03: return "\\x03";
		case 0x04: return "\\x04";
		case 0x05: return "\\x05";
		case 0x06: return "\\x06";
		case 0x07: return "\\a";
		case 0x08: return "\\b";
		case 0x09: return "\\t";
		case 0x0A: return "\\n";
		case 0x0B: return "\\v";
		case 0x0C: return "\\f";
		case 0x0D: return "\\r";
		case 0x0E: return "\\x0E";
		case 0x0F: return "\\x0F";
		case 0x10: return "\\x10";
		case 0x11: return "\\x11";
		case 0x12: return "\\x12";
		case 0x13: return "\\x13";
		case 0x14: return "\\x14";
		case 0x15: return "\\x15";
		case 0x16: return "\\x16";
		case 0x17: return "\\x17";
		case 0x18: return "\\x18";
		case 0x19: return "\\x19";
		case 0x1A: return "\\x1A";
		case 0x1B: return "\\x1B";
		case 0x1C: return "\\x1C";
		case 0x1D: return "\\x1D";
		case 0x1E: return "\\x1E";
		case 0x1F: return "\\x1F";
		case 0x7F: return "\\x7F";
		default:
			return NULL;
	}
}

/* PUBLIC FUNCTIONS */

char * concatenate(const unsigned int count, ...) {
	va_list arguments;
	va_start(arguments, count);
	unsigned int length = 1;
	for (unsigned int k = 0; k < count; ++k) {
		const char * nextString = va_arg(arguments, const char *);
		length += strlen(nextString);
	}
	va_end(arguments);
	char * string = calloc(length, sizeof(char));
	va_start(arguments, count);
	for (unsigned int k = 0; k < count; ++k) {
		const char * nextString = va_arg(arguments, const char *);
		strcat(string, nextString);
	}
	va_end(arguments);
	return string;
}

char * escape(const char * string) {
	unsigned int length = 1;
	for (unsigned int k = 0; string[k] != '\0'; ++k) {
		const unsigned char byte = (unsigned char) string[k];
		if (iscntrl(byte)) {
			length += strlen(_controlCharacterToEscapedString(byte));
		}
		else {
			length += 1;
		}
	}
	char * escapedString = calloc(length, sizeof(char));
	char charToString[2] = { 0, 0 };
	for (unsigned int k = 0; string[k] != '\0'; ++k) {
		const unsigned char byte = (unsigned char) string[k];
		if (iscntrl(byte)) {
			strcat(escapedString, _controlCharacterToEscapedString(byte));
		}
		else {
			charToString[0] = string[k];
			strcat(escapedString, charToString);
		}
	}
	return escapedString;
}

char * indentation(const char character, const unsigned int level, const unsigned int size) {
	const unsigned int indentationLength = level * size;
	char * indentation = calloc(1 + indentationLength, sizeof(char));
	for (unsigned int k = 0; k < indentationLength; ++k) {
		indentation[k] = character;
	}
	return indentation;
}

char * normalizeCategory(const char * raw) {
	const size_t length = strlen(raw);
	// una secuencia UTF-8 de 2 bytes colapsa a 1, asi que length alcanza de sobra
	char * normalized = malloc(length + 1);
	size_t out = 0;
	for (size_t in = 0; in < length;) {
		const unsigned char byte = (unsigned char) raw[in];
		if (byte == 0xC3 && in + 1 < length) {
			const unsigned char tail = (unsigned char) raw[in + 1];
			char folded = '\0';
			switch (tail) {
				case 0xA1: case 0x81: folded = 'a'; break; // á Á
				case 0xA9: case 0x89: folded = 'e'; break; // é É
				case 0xAD: case 0x8D: folded = 'i'; break; // í Í
				case 0xB3: case 0x93: folded = 'o'; break; // ó Ó
				case 0xBA: case 0x9A: case 0xBC: case 0x9C: folded = 'u'; break; // ú Ú ü Ü
				case 0xB1: case 0x91: folded = 'n'; break; // ñ Ñ
				default: break;
			}
			if (folded != '\0') {
				normalized[out++] = folded;
				in += 2;
				continue;
			}
		}
		normalized[out++] = (char) tolower(byte);
		in += 1;
	}
	normalized[out] = '\0';
	return normalized;
}
