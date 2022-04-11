#pragma once

#define TOKEN_TYPES_X \
	X(ILLEGAL, "ILLEGAL") \
	X(END_OF_FILE, "EOF") \
	X(IDENT, "IDENT") \
	X(INT, "INT") \
	X(ASSIGN, "=") \
	X(PLUS, "+") \
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
 * @brief Returns the string representation of the given token type.
 */
const char* TokenTypeText(TokenType type);

/**
 * @brief Destroys the resources held by the given token.
 */
void DestroyToken(Token* token);
