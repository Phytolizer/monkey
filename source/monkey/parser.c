#include "monkey/parser.h"

#include "buffer.h"
#include "monkey/string.h"

#include <stdlib.h>

struct Parser {
	Lexer* lexer;

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

MONKEY_FILE_LOCAL bool expectPeek(Parser* parser, TokenType t) {
	if (peekTokenIs(parser, t)) {
		nextToken(parser);
		return true;
	}

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

MONKEY_FILE_LOCAL Statement* parseStatement(Parser* parser) {
	switch (parser->currentToken.type) {
		case TOKEN_TYPE_LET:
			return parseLetStatement(parser);
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
	free(parser);
}