#pragma once

#include "monkey.h"

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
	X(COMMA, ",") \
	X(SEMICOLON, ";") \
	X(LPAREN, "(") \
	X(RPAREN, ")") \
	X(LBRACE, "{") \
	X(RBRACE, "}") \
	X(FUNCTION, "FUNCTION") \
	X(LET, "LET")

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
 * @brief Creates a new token state.
 *
 * @return A new token state.
 */
MonkeyTokenState* CreateTokenState(void);

/**
 * @brief Destroys a token state.
 *
 * @param state The token state to destroy.
 */
void DestroyTokenState(MonkeyTokenState* state);

/**
 * @brief Returns the string representation of the given token type.
 */
const char* TokenTypeText(TokenType type);

/**
 * @brief Destroys the resources held by the given token.
 */
void DestroyToken(Token* token);

/**
 * @brief Looks up the identifier in the keywords table and returns the
 * corresponding token type.
 *
 * @param identifier The identifier to look up.
 * @return The token type corresponding to the identifier.
 */
TokenType LookupIdent(Monkey* monkey, const char* identifier);
