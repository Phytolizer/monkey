#include "monkey/lexer.h"

#include "monkey/macros.h"
#include "monkey/string.h"

#include <stdbool.h>
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

MONKEY_INTERNAL bool isLetter(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

MONKEY_INTERNAL bool isDigit(char ch) {
	return ch >= '0' && ch <= '9';
}

MONKEY_INTERNAL char* readIdentifier(Lexer* lexer) {
	size_t position = lexer->position;
	while (isLetter(lexer->ch)) {
		readChar(lexer);
	}
	return MonkeyStrndup(lexer->input + position, lexer->position - position);
}

MONKEY_INTERNAL char* readNumber(Lexer* lexer) {
	size_t position = lexer->position;
	while (isDigit(lexer->ch)) {
		readChar(lexer);
	}
	return MonkeyStrndup(lexer->input + position, lexer->position - position);
}

MONKEY_INTERNAL void skipWhitespace(Lexer* lexer) {
	while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' || lexer->ch == '\r') {
		readChar(lexer);
	}
}

#define NEW_TOKEN(...) newToken((NewTokenArgs){__VA_ARGS__})

Lexer CreateLexer(Monkey* monkey, const char* input) {
	Lexer lexer;
	lexer.monkey = monkey;
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

	skipWhitespace(lexer);

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
		default:
			if (isLetter(lexer->ch)) {
				tok.literal = readIdentifier(lexer);
				tok.type = LookupIdent(lexer->monkey, tok.literal);
				return tok;
			} else if (isDigit(lexer->ch)) {
				tok.type = TOKEN_TYPE_INT;
				tok.literal = readNumber(lexer);
				return tok;
			} else {
				tok = NEW_TOKEN(.type = TOKEN_TYPE_ILLEGAL, .literal = lexer->ch);
			}
			break;
	}

	readChar(lexer);
	return tok;
}

void DestroyLexer(Lexer* lexer) {
	// Nothing to do here.
	(void)lexer;
}
