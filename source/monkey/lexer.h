#pragma once

#include "monkey.h"
#include "monkey/token.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Lexer is a struct that stores state related to the lexing of Monkey.
 */
typedef struct {
	Monkey* monkey;
	const char* input;
	size_t inputLength;
	uint64_t position;
	uint64_t readPosition;
	char ch;
} Lexer;

/**
 * @brief Creates a new lexer.
 * @param input The input string to lex.
 * @return A new lexer.
 */
Lexer CreateLexer(Monkey* monkey, const char* input);

/**
 * @brief LexerNextToken gets the next token from the lexer.
 * @param lexer The lexer to get the next token from.
 * @return The next token from the lexer.
 */
Token LexerNextToken(Lexer* lexer);

/**
 * @brief DestroyLexer destroys a lexer.
 * @param lexer The lexer to destroy.
 */
void DestroyLexer(Lexer* lexer);
