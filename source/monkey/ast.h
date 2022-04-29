#pragma once

#include "monkey/token.h"
#include "span.h"

#define NODE_TYPES_X \
	X(EXPRESSION) \
	X(STATEMENT) \
	X(PROGRAM)

typedef enum {
#define X(name) NODE_TYPE_##name,
	NODE_TYPES_X
#undef X
} NodeType;

typedef struct {
	NodeType type;
} Node;

#define STATEMENT_TYPES_X \
	X(LET) \
	X(RETURN)

typedef enum {
#define X(name) STATEMENT_TYPE_##name,
	STATEMENT_TYPES_X
#undef X
} StatementType;

typedef struct {
	Node base;
	StatementType type;
} Statement;

char* StatementTokenLiteral(const Statement* statement);

#define EXPRESSION_TYPES_X X(IDENTIFIER)

typedef enum {
#define X(name) EXPRESSION_TYPE_##name,
	EXPRESSION_TYPES_X
#undef X
} ExpressionType;

typedef struct {
	Node base;
	ExpressionType type;
} Expression;

typedef SPAN_TYPE(Statement*) StatementSpan;

typedef struct {
	Node base;
	StatementSpan statements;
} Program;

Program* CreateProgram(StatementSpan statements);
char* ProgramTokenLiteral(const Program* program);
void DestroyProgram(Program* program);

typedef struct {
	Expression base;
	Token token;
	char* value;
} Identifier;

Identifier* CreateIdentifier(Token token, char* value);
char* IdentifierTokenLiteral(const Identifier* identifier);
void DestroyIdentifier(Identifier* identifier);

typedef struct {
	Statement base;
	Token token;
	Identifier* identifier;
	Expression* value;
} LetStatement;

LetStatement* CreateLetStatement(Token token, Identifier* identifier, Expression* value);
char* LetStatementTokenLiteral(const LetStatement* statement);

typedef struct {
	Statement base;
	Token token;
	Expression* returnValue;
} ReturnStatement;

ReturnStatement* CreateReturnStatement(Token token, Expression* returnValue);
char* ReturnStatementTokenLiteral(const ReturnStatement* statement);
