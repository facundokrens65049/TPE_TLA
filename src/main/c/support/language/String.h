#ifndef STRING_HEADER
#define STRING_HEADER

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/**
 * Concatenates a list of strings. The returned string must be freed because
 * it uses heap-memory.
 *
 * @param count
 *	The size of the list.
 */
char * concatenate(const unsigned int count, ...);

/**
 * Creates a new string (using heap-memory), that is equal to the original,
 * but using escaped sequences for every control character (i.e., in the range
 * 0x00-0x1F and 0x7F).
 */
char * escape(const char * string);

/**
 * Generates an indentation string for the specified level, using heap-memory.
 *
 * @param character
 *	The character to use for spacing, for example ' ' or '\t'.
 * @param level
 *	The depth level to indent. Zero (0) means no-indentation.
 * @param size
 *	The size of every indentation level, thus, level N as a length of N*size
 *	characters.
 */
char * indentation(const char character, const unsigned int level, const unsigned int size);

/**
 * Normalizes a category identifier to lowercase ASCII without accents. The
 * grammar allows UTF-8 letters from the Latin-1 supplement block (lead byte
 * 0xC3), so the common Spanish accented vowels and "ñ" are folded to their
 * base letter; every other byte is lowercased as ASCII. The returned string
 * uses heap-memory and must be freed.
 *
 * This is the single source of truth shared by the semantic analyzer (which
 * fills the symbol table) and the code generator (which emits the categories).
 */
char * normalizeCategory(const char * raw);

#endif
