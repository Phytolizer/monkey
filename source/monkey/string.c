#include "monkey/string.h"

#include <stdlib.h>
#include <string.h>

char* MonkeyStrdup(const char* str) {
	size_t len = strlen(str);
	char* result = malloc(sizeof(char) * (len + 1));
	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}
