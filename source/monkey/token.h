#pragma once

#include "monkey.h"

/**
 * @brief TOKEN_TYPES_X is a list of all the token types.
 */
#define TOKEN_TYPES_X \
	X(ILLEGAL, "ILLEGAL") \
	X(END_OF_FILE, "EOF") \
	X(IDENT, "IDENT") \
	X(INT, "INT") \
	X(ASSIGN, "=") \
	X(PLUS, "+") \
	X(MINUS, "-") \
	X(BANG, "!") \
	X(ASTERISK, "*") \
	X(SLASH, "/") \
	X(LT, "<") \
	X(GT, ">") \
	X(EQ, "==") \
	X(NOT_EQ, "!=") \
	X(COMMA, ",") \
	X(SEMICOLON, ";") \
	X(LPAREN, "(") \
	X(RPAREN, ")") \
	X(LBRACE, "{") \
	X(RBRACE, "}") \
	X(ELSE, "ELSE") \
	X(FALSE, "FALSE") \
	X(FUNCTION, "FUNCTION") \
	X(IF, "IF") \
	X(LET, "LET") \
	X(RETURN, "RETURN") \
	X(TRUE, "TRUE")

/**
 * @brief TokenType is an enumeration of all the different types of tokens
 * that can be found in Monkey.
 */
typedef enum {
#define X(name, string) TOKEN_TYPE_##name,
	TOKEN_TYPES_X
#undef X
} TokenType;

/**
 * @brief Token is a struct that holds information about a token.
 */
typedef struct {
	TokenType type;
	char* literal;
} Token;

/**
 * @private
 */
MONKEY_INTERNAL MonkeyTokenState* CreateTokenState(void);

/**
 * @private
 */
MONKEY_INTERNAL void DestroyTokenState(MonkeyTokenState* state);

/**
 * @brief Returns the string representation of the given token type.
 */
const char* TokenTypeText(TokenType type);

/**
 * @brief Destroys the resources held by the given token.
 */
void DestroyToken(Token* token);

/**
 * @private
 */
MONKEY_INTERNAL TokenType LookupIdent(Monkey* monkey, const char* identifier);
