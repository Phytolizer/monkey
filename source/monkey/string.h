#pragma once

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
