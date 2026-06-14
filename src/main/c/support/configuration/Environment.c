#include "Environment.h"

/* PUBLIC FUNCTIONS */

bool getBooleanOrDefault(const char * name, const bool defaultValue) {
	const char * value = getStringOrDefault(name, NULL);
	if (value == NULL) {
		return defaultValue;
	}
	if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
		return true;
	}
	if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
		return false;
	}
	// Variable present but with an unrecognized value (typo, "yes", etc.):
	// we fall back to the default instead of silently assuming false.
	return defaultValue;
}

const char * getStringOrDefault(const char * name, const char * defaultValue) {
	const char * value = getenv(name);
	if (value == NULL) {
		return defaultValue;
	}
	else {
		return value;
	}
}
