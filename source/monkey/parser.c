#include "monkey/parser.h"

#include "buffer.h"
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/macros.h"
#include "monkey/string.h"
#include "monkey/token.h"

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
		case TOKEN_TYPE_LPAREN:
			return PRECEDENCE_CALL;
		default:
			return PRECEDENCE_LOWEST;
	}
}

MONKEY_FILE_LOCAL PrefixParseFn* getPrefixParser(TokenType type);
MONKEY_FILE_LOCAL InfixParseFn* getInfixParser(TokenType type);

struct Parser {
	Lexer* lexer;

	MonkeyStringBuffer errors;

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
	BUFFER_PUSH(&parser->errors, message);
}

MONKEY_FILE_LOCAL void noPrefixParseFnError(Parser* parser, TokenType t) {
	char* message = MonkeyAsprintf("no prefix parse function for %s found", TokenTypeText(t));
	BUFFER_PUSH(&parser->errors, message);
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
		BUFFER_PUSH(&parser->errors, message);
		return NULL;
	}
	return (Expression*)CreateIntegerLiteral(token, value);
}

MONKEY_FILE_LOCAL Expression* parseBoolean(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	return (Expression*)CreateBooleanLiteral(token, curTokenIs(parser, TOKEN_TYPE_TRUE));
}

MONKEY_FILE_LOCAL Expression* parsePrefixExpression(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);
	char* op = MonkeyStrdup(token.literal);

	nextToken(parser);

	Expression* right = parseExpression(parser, PRECEDENCE_PREFIX);

	return (Expression*)CreatePrefixExpression(token, op, right);
}

MONKEY_FILE_LOCAL Expression* parseGroupedExpression(Parser* parser) {
	nextToken(parser);

	Expression* exp = parseExpression(parser, PRECEDENCE_LOWEST);

	if (!expectPeek(parser, TOKEN_TYPE_RPAREN)) {
		DestroyExpression(exp);
		return NULL;
	}

	return exp;
}

MONKEY_FILE_LOCAL BlockStatement* parseBlockStatement(Parser* parser);

MONKEY_FILE_LOCAL Expression* parseIfExpression(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	if (!expectPeek(parser, TOKEN_TYPE_LPAREN)) {
		DestroyToken(&token);
		return NULL;
	}

	nextToken(parser);
	Expression* condition = parseExpression(parser, PRECEDENCE_LOWEST);
	if (!expectPeek(parser, TOKEN_TYPE_RPAREN) || !expectPeek(parser, TOKEN_TYPE_LBRACE)) {
		DestroyExpression(condition);
		DestroyToken(&token);
		return NULL;
	}

	BlockStatement* consequence = parseBlockStatement(parser);

	BlockStatement* alternative = NULL;

	if (peekTokenIs(parser, TOKEN_TYPE_ELSE)) {
		nextToken(parser);
		if (!expectPeek(parser, TOKEN_TYPE_LBRACE)) {
			DestroyBlockStatement(consequence);
			DestroyExpression(condition);
			DestroyToken(&token);
			return NULL;
		}

		alternative = parseBlockStatement(parser);
	}

	return (Expression*)CreateIfExpression(token, condition, consequence, alternative);
}

MONKEY_FILE_LOCAL bool parseFunctionParameters(Parser* parser, IdentifierSpan* outParameters) {
	IdentifierBuffer identifiers = BUFFER_INIT;

	if (peekTokenIs(parser, TOKEN_TYPE_RPAREN)) {
		nextToken(parser);
		*outParameters = (IdentifierSpan)BUFFER_AS_SPAN(identifiers);
		return true;
	}

	nextToken(parser);
	BUFFER_PUSH(&identifiers,
			CreateIdentifier(
					CopyToken(&parser->currentToken), MonkeyStrdup(parser->currentToken.literal)));

	while (peekTokenIs(parser, TOKEN_TYPE_COMMA)) {
		nextToken(parser);
		nextToken(parser);
		BUFFER_PUSH(&identifiers,
				CreateIdentifier(CopyToken(&parser->currentToken),
						MonkeyStrdup(parser->currentToken.literal)));
	}

	if (!expectPeek(parser, TOKEN_TYPE_RPAREN)) {
		for (size_t i = 0; i < identifiers.length; ++i) {
			DestroyIdentifier(identifiers.data[i]);
		}
		BUFFER_FREE(identifiers);
		return false;
	}

	*outParameters = (IdentifierSpan)BUFFER_AS_SPAN(identifiers);
	return true;
}

MONKEY_FILE_LOCAL Expression* parseFunctionLiteral(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	if (!expectPeek(parser, TOKEN_TYPE_LPAREN)) {
		DestroyToken(&token);
		return NULL;
	}

	IdentifierSpan parameters;
	if (!parseFunctionParameters(parser, &parameters)) {
		DestroyToken(&token);
		return NULL;
	}

	if (!expectPeek(parser, TOKEN_TYPE_LBRACE)) {
		for (size_t i = 0; i < parameters.length; ++i) {
			DestroyIdentifier(parameters.begin[i]);
		}
		DestroyToken(&token);
		return NULL;
	}

	BlockStatement* body = parseBlockStatement(parser);

	return (Expression*)CreateFunctionLiteral(token, parameters, body);
}

MONKEY_FILE_LOCAL Expression* parseInfixExpression(Parser* parser, Expression* left) {
	Token token = CopyToken(&parser->currentToken);
	char* op = MonkeyStrdup(token.literal);

	Precedence precedence = curPrecedence(parser);
	nextToken(parser);
	Expression* right = parseExpression(parser, precedence);

	return (Expression*)CreateInfixExpression(token, left, op, right);
}

MONKEY_FILE_LOCAL bool parseCallArguments(Parser* parser, ExpressionSpan* outArguments) {
	ExpressionBuffer arguments = BUFFER_INIT;

	if (peekTokenIs(parser, TOKEN_TYPE_RPAREN)) {
		nextToken(parser);
		*outArguments = (ExpressionSpan)BUFFER_AS_SPAN(arguments);
		return true;
	}

	nextToken(parser);
	BUFFER_PUSH(&arguments, parseExpression(parser, PRECEDENCE_LOWEST));

	while (peekTokenIs(parser, TOKEN_TYPE_COMMA)) {
		nextToken(parser);
		nextToken(parser);
		BUFFER_PUSH(&arguments, parseExpression(parser, PRECEDENCE_LOWEST));
	}

	if (!expectPeek(parser, TOKEN_TYPE_RPAREN)) {
		for (size_t i = 0; i < arguments.length; ++i) {
			DestroyExpression(arguments.data[i]);
		}
		BUFFER_FREE(arguments);
		return false;
	}

	*outArguments = (ExpressionSpan)BUFFER_AS_SPAN(arguments);
	return true;
}

MONKEY_FILE_LOCAL Expression* parseCallExpression(Parser* parser, Expression* function) {
	Token token = CopyToken(&parser->currentToken);

	ExpressionSpan arguments;
	if (!parseCallArguments(parser, &arguments)) {
		DestroyToken(&token);
		return NULL;
	}

	return (Expression*)CreateCallExpression(token, function, arguments);
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

MONKEY_FILE_LOCAL Statement* parseStatement(Parser* parser);

MONKEY_FILE_LOCAL BlockStatement* parseBlockStatement(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);
	StatementBuffer statements = BUFFER_INIT;

	nextToken(parser);

	while (!curTokenIs(parser, TOKEN_TYPE_RBRACE) && !curTokenIs(parser, TOKEN_TYPE_END_OF_FILE)) {
		Statement* stmt = parseStatement(parser);
		if (stmt != NULL) {
			BUFFER_PUSH(&statements, stmt);
		}
		nextToken(parser);
	}

	return CreateBlockStatement(token, (StatementSpan)BUFFER_AS_SPAN(statements));
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

	nextToken(parser);
	Expression* value = parseExpression(parser, PRECEDENCE_LOWEST);
	if (peekTokenIs(parser, TOKEN_TYPE_SEMICOLON)) {
		nextToken(parser);
	}

	return (Statement*)CreateLetStatement(token, name, value);
}

MONKEY_FILE_LOCAL Statement* parseReturnStatement(Parser* parser) {
	Token token = CopyToken(&parser->currentToken);

	nextToken(parser);

	Expression* returnValue = parseExpression(parser, PRECEDENCE_LOWEST);
	if (peekTokenIs(parser, TOKEN_TYPE_SEMICOLON)) {
		nextToken(parser);
	}

	return (Statement*)CreateReturnStatement(token, returnValue);
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
	for (size_t i = 0; i < parser->errors.length; i++) {
		free(parser->errors.data[i]);
	}
	BUFFER_FREE(parser->errors);
	free(parser);
}

MonkeyStringBuffer ParserErrors(Parser* parser) {
	return parser->errors;
}

MONKEY_FILE_LOCAL PrefixParseFn* getPrefixParser(TokenType type) {
	switch (type) {
		case TOKEN_TYPE_IDENT:
			return &parseIdentifier;
		case TOKEN_TYPE_INT:
			return &parseIntegerLiteral;
		case TOKEN_TYPE_TRUE:
		case TOKEN_TYPE_FALSE:
			return &parseBoolean;
		case TOKEN_TYPE_BANG:
		case TOKEN_TYPE_MINUS:
			return &parsePrefixExpression;
		case TOKEN_TYPE_LPAREN:
			return &parseGroupedExpression;
		case TOKEN_TYPE_IF:
			return &parseIfExpression;
		case TOKEN_TYPE_FUNCTION:
			return &parseFunctionLiteral;
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
		case TOKEN_TYPE_LPAREN:
			return &parseCallExpression;
		default:
			return NULL;
	}
}
