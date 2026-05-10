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
	for (unsigned int k = 0; 0 < string[k]; ++k) {
		if (iscntrl(string[k])) {
			length += strlen(_controlCharacterToEscapedString(string[k]));
		}
		else {
			length += 1;
		}
	}
	char * escapedString = calloc(length, sizeof(char));
	char charToString[2] = { 0, 0 };
	for (unsigned int k = 0; 0 < string[k]; ++k) {
		if (iscntrl(string[k])) {
			strcat(escapedString, _controlCharacterToEscapedString(string[k]));
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
	for (int k = 0; k < indentationLength; ++k) {
		indentation[k] = character;
	}
	return indentation;
}

char * normalizeFinancialCategory(const char * raw) {
	if (raw == NULL) {
		return NULL;
	}
	const size_t cap = strlen(raw) * 3u + 1u;
	char * const out = calloc(cap, sizeof(char));
	if (out == NULL) {
		return strdup(raw);
	}
	char * w = out;
	const unsigned char * s = (const unsigned char *) raw;
	while (*s != '\0') {
		if (*s < 128u) {
			*w++ = (char) (isupper((int) *s) ? tolower((int) *s) : (int) *s);
			s++;
			continue;
		}
		if (*s == 0xC3u && s[1] != '\0') {
			const unsigned char c2 = s[1];
			/* Preserve ñ / Ñ as UTF-8 lowercase ñ (U+00F1). */
			if (c2 == 0xB1u || c2 == 0x91u) {
				*w++ = (char) 0xC3u;
				*w++ = (char) 0xB1u;
				s += 2;
				continue;
			}
			char mapped = '\0';
			switch (c2) {
				case 0xA1u: case 0xA0u: case 0xA2u: case 0xA3u: mapped = 'a'; break;
				case 0xA9u: case 0xA8u: case 0xAAu: case 0xABu: mapped = 'e'; break;
				case 0xADu: case 0xACu: case 0xAEu: case 0xAFu: mapped = 'i'; break;
				case 0xB3u: case 0xB2u: case 0xB4u: case 0xB5u: mapped = 'o'; break;
				case 0xBAu: case 0xB9u: case 0xBBu: mapped = 'u'; break;
				case 0xBCu: mapped = 'u'; break;
				case 0x81u: case 0x80u: case 0x82u: case 0x83u: mapped = 'a'; break;
				case 0x89u: case 0x88u: case 0x8Au: case 0x8Bu: mapped = 'e'; break;
				case 0x8Du: case 0x8Cu: case 0x8Eu: case 0x8Fu: mapped = 'i'; break;
				case 0x93u: case 0x92u: case 0x94u: case 0x95u: mapped = 'o'; break;
				case 0x9Au: case 0x99u: case 0x9Bu: case 0x9Cu: mapped = 'u'; break;
				case 0xA7u: case 0x87u: mapped = 'c'; break;
				default: break;
			}
			if (mapped != '\0') {
				*w++ = mapped;
				s += 2;
				continue;
			}
		}
		*w++ = (char) *s++;
	}
	*w = '\0';
	return out;
}
