#include "monkey/string.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* MonkeyStrdup(const char* str) {
	size_t len = strlen(str);
	char* result = malloc(sizeof(char) * (len + 1));
	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}

char* MonkeyStrndup(const char* str, size_t n) {
	char* result = malloc(sizeof(char) * (n + 1));
	memcpy(result, str, n);
	result[n] = '\0';
	return result;
}

char* MonkeyAsprintf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	va_end(args);

	if (len < 0) {
		return NULL;
	}

	va_start(args, format);
	char* result = malloc(sizeof(char) * ((size_t)len + 1));
	(void)vsnprintf(result, (size_t)len + 1, format, args);
	va_end(args);

	return result;
}
