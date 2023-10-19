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
#include <stdint.h>
#include <stdlib.h>

typedef Expression*(PrefixParseFn)(Parser*);
typedef Expression*(InfixParseFn)(Parser*, Expression*);

#define PRECEDENCES_X \
	X(LOWEST) \
	X(EQUALS) \
	X(LESSGREATER) \
	X(SUM) \
	X(PRODUCT) \
	X(PREFIX) \
	X(CALL)

typedef enum {
#define X(x) PRECEDENCE_##x,
	PRECEDENCES_X
#undef X
} Precedence;

MONKEY_FILE_LOCAL Precedence getInfixPrecedence(TokenType type) {
	switch (type) {
		case TOKEN_TYPE_EQ:
		case TOKEN_TYPE_NOT_EQ:
			return PRECEDENCE_EQUALS;
		case TOKEN_TYPE_LT:
		case TOKEN_TYPE_GT:
			return PRECEDENCE_LESSGREATER;
		case TOKEN_TYPE_PLUS:
		case TOKEN_TYPE_MINUS:
			return PRECEDENCE_SUM;
		case TOKEN_TYPE_ASTERISK:
		case TOKEN_TYPE_SLASH:
			return PRECEDENCE_PRODUCT;
		default:
			return PRECEDENCE_LOWEST;
	}
}

MONKEY_FILE_LOCAL PrefixParseFn* getPrefixParser(TokenType type);
MONKEY_FILE_LOCAL InfixParseFn* getInfixParser(TokenType type);

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

MONKEY_FILE_LOCAL Precedence curPrecedence(Parser* parser) {
	return getInfixPrecedence(parser->currentToken.type);
}

MONKEY_FILE_LOCAL Precedence peekPrecedence(Parser* parser) {
	return getInfixPrecedence(parser->peekToken.type);
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

MONKEY_FILE_LOCAL void noPrefixParseFnError(Parser* parser, TokenType t) {
	char* message = MonkeyAsprintf("no prefix parse function for %s found", TokenTypeText(t));
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

MONKEY_FILE_LOCAL Expression* parseExpression(Parser* parser, Precedence precedence);

MONKEY_FILE_LOCAL Expression* parseIdentifier(Parser* parser) {
	return (Expression*)CreateIdentifier(
			CopyToken(&parser->currentToken), MonkeyStrdup(parser->currentToken.literal));
}

MONKEY_FILE_LOCAL Expression* parseIntegerLiteral(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	char* errptr = NULL;
	enum { BASE_10 = 10 };
	int64_t value = strtoll(token.literal, &errptr, BASE_10);
	if (errptr != NULL && *errptr != '\0') {
		char* message = MonkeyAsprintf("could not parse \"%s\" as integer", token.literal);
		VECTOR_PUSH(&parser->errors, message);
		return NULL;
	}
	return (Expression*)CreateIntegerLiteral(token, value);
}

MONKEY_FILE_LOCAL Expression* parsePrefixExpression(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);
	char* op = MonkeyStrdup(token.literal);

	nextToken(parser);

	Expression* right = parseExpression(parser, PRECEDENCE_PREFIX);

	return (Expression*)CreatePrefixExpression(token, op, right);
}

MONKEY_FILE_LOCAL Expression* parseInfixExpression(Parser* parser, Expression* left) {
	Token token = CopyToken(&parser->currentToken);
	char* op = MonkeyStrdup(token.literal);

	Precedence precedence = curPrecedence(parser);
	nextToken(parser);
	Expression* right = parseExpression(parser, precedence);

	return (Expression*)CreateInfixExpression(token, left, op, right);
}

MONKEY_FILE_LOCAL Expression* parseExpression(Parser* parser, Precedence precedence) {
	PrefixParseFn* prefix = getPrefixParser(parser->currentToken.type);
	if (prefix == NULL) {
		noPrefixParseFnError(parser, parser->currentToken.type);
		return NULL;
	}

	Expression* leftExp = prefix(parser);

	while (!peekTokenIs(parser, TOKEN_TYPE_SEMICOLON) && precedence < peekPrecedence(parser)) {
		InfixParseFn* infix = getInfixParser(parser->peekToken.type);
		if (infix == NULL) {
			break;
		}

		nextToken(parser);

		leftExp = infix(parser, leftExp);
	}

	return leftExp;
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

MONKEY_FILE_LOCAL Statement* parseExpressionStatement(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	Expression* expression = parseExpression(parser, PRECEDENCE_LOWEST);

	if (peekTokenIs(parser, TOKEN_TYPE_SEMICOLON)) {
		nextToken(parser);
	}

	return (Statement*)CreateExpressionStatement(token, expression);
}

MONKEY_FILE_LOCAL Statement* parseStatement(Parser* parser) {
	switch (parser->currentToken.type) {
		case TOKEN_TYPE_LET:
			return parseLetStatement(parser);
		case TOKEN_TYPE_RETURN:
			return parseReturnStatement(parser);
		default:
			return parseExpressionStatement(parser);
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

MONKEY_FILE_LOCAL PrefixParseFn* getPrefixParser(TokenType type) {
	switch (type) {
		case TOKEN_TYPE_IDENT:
			return &parseIdentifier;
		case TOKEN_TYPE_INT:
			return &parseIntegerLiteral;
		case TOKEN_TYPE_BANG:
		case TOKEN_TYPE_MINUS:
			return &parsePrefixExpression;
		default:
			return NULL;
	}
}

MONKEY_FILE_LOCAL InfixParseFn* getInfixParser(TokenType type) {
	switch (type) {
		case TOKEN_TYPE_PLUS:
		case TOKEN_TYPE_MINUS:
		case TOKEN_TYPE_ASTERISK:
		case TOKEN_TYPE_SLASH:
		case TOKEN_TYPE_EQ:
		case TOKEN_TYPE_NOT_EQ:
		case TOKEN_TYPE_LT:
		case TOKEN_TYPE_GT:
			return &parseInfixExpression;
		default:
			return NULL;
	}
}
