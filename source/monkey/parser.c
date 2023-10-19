#include "monkey/parser.h"

#include "buffer.h"
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/macros.h"
#include "monkey/string.h"
#include "monkey/token.h"
#include "monkey/vector.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

struct Parser {
	Lexer* lexer;

	MonkeyStringVector errors;

	Token currentToken;
	Token peekToken;
};

typedef BUFFER_TYPE(Statement*) StatementBuffer;

MONKEY_FILE_LOCAL void nextToken(Parser* parser) {
	DestroyToken(&parser->currentToken);
	parser->currentToken = parser->peekToken;
	parser->peekToken = LexerNextToken(parser->lexer);
}

MONKEY_FILE_LOCAL bool curTokenIs(Parser* parser, TokenType type) {
	return parser->currentToken.type == type;
}

MONKEY_FILE_LOCAL bool peekTokenIs(Parser* parser, TokenType type) {
	return parser->peekToken.type == type;
}

MONKEY_FILE_LOCAL void peekError(Parser* parser, TokenType t) {
	char* message = MonkeyAsprintf("expected next token to be %s, got %s instead", TokenTypeText(t),
			TokenTypeText(parser->peekToken.type));
	VECTOR_PUSH(&parser->errors, message);
}

MONKEY_FILE_LOCAL bool expectPeek(Parser* parser, TokenType t) {
	if (peekTokenIs(parser, t)) {
		nextToken(parser);
		return true;
	}

	peekError(parser, t);
	return false;
}

Parser* CreateParser(Lexer* lexer) {
	Parser* parser = calloc(1, sizeof(Parser));
	parser->lexer = lexer;

	nextToken(parser);
	nextToken(parser);

	return parser;
}

MONKEY_FILE_LOCAL Statement* parseLetStatement(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	if (!expectPeek(parser, TOKEN_TYPE_IDENT)) {
		DestroyToken(&token);
		return NULL;
	}

	Identifier* name = CreateIdentifier(
			CopyToken(&parser->currentToken), MonkeyStrdup(parser->currentToken.literal));

	if (!expectPeek(parser, TOKEN_TYPE_ASSIGN)) {
		DestroyToken(&token);
		DestroyIdentifier(name);
		return NULL;
	}

	// TODO: parse expression
	while (!curTokenIs(parser, TOKEN_TYPE_SEMICOLON)) {
		nextToken(parser);
	}

	return (Statement*)CreateLetStatement(token, name, NULL);
}

MONKEY_FILE_LOCAL Statement* parseReturnStatement(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	nextToken(parser);

	// TODO: parse expression
	while (!curTokenIs(parser, TOKEN_TYPE_SEMICOLON)) {
		nextToken(parser);
	}

	return (Statement*)CreateReturnStatement(token, NULL);
}

MONKEY_FILE_LOCAL Statement* parseStatement(Parser* parser) {
	switch (parser->currentToken.type) {
		case TOKEN_TYPE_LET:
			return parseLetStatement(parser);
		case TOKEN_TYPE_RETURN:
			return parseReturnStatement(parser);
		default:
			return NULL;
	}
}

Program* ParseProgram(Parser* parser) {
	StatementBuffer statements = BUFFER_INIT;

	while (parser->currentToken.type != TOKEN_TYPE_END_OF_FILE) {
		Statement* statement = parseStatement(parser);
		if (statement != NULL) {
			BUFFER_PUSH(&statements, statement);
		}
		nextToken(parser);
	}

	return CreateProgram((StatementSpan)BUFFER_AS_SPAN(statements));
}

void DestroyParser(Parser* parser) {
	DestroyToken(&parser->currentToken);
	DestroyToken(&parser->peekToken);
	for (size_t i = 0; i < parser->errors.size; i++) {
		free(parser->errors.data[i]);
	}
	VECTOR_FREE(&parser->errors);
	free(parser);
}

MonkeyStringVector ParserErrors(Parser* parser) {
	return parser->errors;
}
