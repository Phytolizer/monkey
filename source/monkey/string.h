#pragma once

#include "buffer.h"
#include "span.h"

#include <stdarg.h>
#include <stddef.h>

/**
 * @brief Allocates a new string and copies the given string into it.
 * @param str The string to copy.
 * @return A new string.
 */
char* MonkeyStrdup(const char* str);

/**
 * @brief Allocates a new string and copies the given characters into it.
 * @param str The start of the characters to copy.
 * @param n The number of characters to copy.
 * @return A new string.
 */
char* MonkeyStrndup(const char* str, size_t n);

/**
 * @brief Allocates a new string and formats the given arguments into it.
 * @param format The format string.
 * @param ... The arguments to format.
 * @return A new string.
 */
char* MonkeyAsprintf(const char* format, ...);

/**
 * @brief Allocates a new string and formats the given arguments into it.
 * @param format The format string.
 * @param args The arguments to format.
 * @return A new string.
 */
char* MonkeyAvsprintf(const char* format, va_list args);

/**
 * @brief A collection of strings.
 */
typedef BUFFER_TYPE(char*) MonkeyStringBuffer;

/**
 * @brief A view into a collection of strings.
 */
typedef SPAN_TYPE(char*) MonkeyStringSpan;

char* MonkeyStringJoin(MonkeyStringSpan strings);
