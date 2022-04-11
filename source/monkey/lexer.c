#include "monkey/lexer.h"

#include "monkey/macros.h"
#include "monkey/string.h"

#include <stdlib.h>
#include <string.h>

MONKEY_INTERNAL void readChar(Lexer* lexer) {
	if (lexer->readPosition >= lexer->inputLength) {
		lexer->ch = 0;
	} else {
		lexer->ch = lexer->input[lexer->readPosition];
	}
	lexer->position = lexer->readPosition;
	lexer->readPosition += 1;
}

typedef struct {
	TokenType type;
	char literal;
} NewTokenArgs;

MONKEY_INTERNAL Token newToken(NewTokenArgs args) {
	Token token;
	token.type = args.type;
	token.literal = malloc(sizeof(char) * 2);
	token.literal[0] = args.literal;
	token.literal[1] = '\0';
	return token;
}

#define NEW_TOKEN(...) newToken((NewTokenArgs){__VA_ARGS__})

Lexer CreateLexer(const char* input) {
	Lexer lexer;
	lexer.input = input;
	lexer.inputLength = strlen(input);
	lexer.position = 0;
	lexer.readPosition = 0;
	lexer.ch = '\0';
	readChar(&lexer);
	return lexer;
}

Token LexerNextToken(Lexer* lexer) {
	Token tok;

	switch (lexer->ch) {
		case '=':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_ASSIGN, .literal = lexer->ch);
			break;
		case ';':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_SEMICOLON, .literal = lexer->ch);
			break;
		case '(':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_LPAREN, .literal = lexer->ch);
			break;
		case ')':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_RPAREN, .literal = lexer->ch);
			break;
		case ',':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_COMMA, .literal = lexer->ch);
			break;
		case '+':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_PLUS, .literal = lexer->ch);
			break;
		case '{':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_LBRACE, .literal = lexer->ch);
			break;
		case '}':
			tok = NEW_TOKEN(.type = TOKEN_TYPE_RBRACE, .literal = lexer->ch);
			break;
		case 0:
			tok.type = TOKEN_TYPE_END_OF_FILE;
			tok.literal = MonkeyStrdup("");
			break;
	}

	readChar(lexer);
	return tok;
}

void DestroyLexer(Lexer* lexer) {
	// Nothing to do here.
	(void)lexer;
}
