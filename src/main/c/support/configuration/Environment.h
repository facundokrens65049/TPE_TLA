#ifndef ENVIRONMENT_HEADER
#define ENVIRONMENT_HEADER

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Analog to "getStringOrDefault", but returning a boolean. "true"/"1" yield
 * true and "false"/"0" yield false; the default value is used both when the
 * variable is undefined and when it holds an unrecognized value.
 */
bool getBooleanOrDefault(const char * name, const bool defaultValue);

/**
 * Gets the value of an environment variable by name, or returns a default
 * value if the variable is undefined.
 * 
 * @see https://cplusplus.com/reference/cstdlib/getenv/
 */
const char * getStringOrDefault(const char * name, const char * defaultValue);

#endif
