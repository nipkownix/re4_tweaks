#ifndef LOG_H
#define LOG_H

/**
 * Private logging utilities.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

struct di_logger {
	FILE *f;
	const char *section;
	bool initialized;
};

void
_di_logger_va_add_failure(struct di_logger *logger, const char fmt[], va_list args);

#endif
