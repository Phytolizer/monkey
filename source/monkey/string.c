#include "monkey/string.h"

#include "monkey/vector.h"

#include <stdarg.h>
#include <stddef.h>
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

char* MonkeyStringJoin(MonkeyStringSpan strings) {
	VECTOR_T(size_t) lengths = VECTOR_INIT;
	size_t totalLength = 0;
	for (size_t i = 0; i < strings.length; ++i) {
		VECTOR_PUSH(&lengths, strlen(strings.begin[i]));
		totalLength += lengths.data[lengths.size - 1];
	}
	char* result = malloc(totalLength + 1);
	size_t currentPos = 0;
	for (size_t i = 0; i < strings.length; ++i) {
		memcpy(result + currentPos, strings.begin[i], lengths.data[i]);
		currentPos += lengths.data[i];
	}
	result[currentPos] = '\0';
	VECTOR_FREE(&lengths);
	return result;
}
