#pragma once

#include "monkey/token.h"
#include "span.h"

#include <stdbool.h>
#include <stdint.h>

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
	X(RETURN) \
	X(EXPRESSION)

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
char* StatementString(const Statement* statement);

#define EXPRESSION_TYPES_X \
	X(IDENTIFIER) \
	X(INTEGER_LITERAL) \
	X(BOOLEAN_LITERAL) \
	X(PREFIX) \
	X(INFIX)

typedef enum {
#define X(name) EXPRESSION_TYPE_##name,
	EXPRESSION_TYPES_X
#undef X
} ExpressionType;

typedef struct {
	Node base;
	ExpressionType type;
} Expression;

void DestroyExpression(Expression* expression);
char* ExpressionString(const Expression* expression);

typedef SPAN_TYPE(Statement*) StatementSpan;

typedef struct {
	Node base;
	StatementSpan statements;
} Program;

Program* CreateProgram(StatementSpan statements);
char* ProgramTokenLiteral(const Program* program);
char* ProgramString(const Program* program);
void DestroyProgram(Program* program);

typedef struct {
	Expression base;
	Token token;
	char* value;
} Identifier;

Identifier* CreateIdentifier(Token token, char* value);
char* IdentifierTokenLiteral(const Identifier* identifier);
char* IdentifierString(const Identifier* identifier);
void DestroyIdentifier(Identifier* identifier);

typedef struct {
	Expression base;
	Token token;
	int64_t value;
} IntegerLiteral;

IntegerLiteral* CreateIntegerLiteral(Token token, int64_t value);
char* IntegerLiteralTokenLiteral(const IntegerLiteral* integerLiteral);
char* IntegerLiteralString(const IntegerLiteral* integerLiteral);
void DestroyIntegerLiteral(IntegerLiteral* integerLiteral);

typedef struct {
	Expression base;
	Token token;
	bool value;
} BooleanLiteral;

BooleanLiteral* CreateBooleanLiteral(Token token, int64_t value);
char* BooleanLiteralTokenLiteral(const BooleanLiteral* booleanLiteral);
char* BooleanLiteralString(const BooleanLiteral* booleanLiteral);
void DestroyBooleanLiteral(BooleanLiteral* booleanLiteral);

typedef struct {
	Expression base;
	Token token;
	char* op;
	Expression* right;
} PrefixExpression;

PrefixExpression* CreatePrefixExpression(Token token, char* op, Expression* right);
char* PrefixExpressionTokenLiteral(const PrefixExpression* prefix);
char* PrefixExpressionString(const PrefixExpression* prefix);
void DestroyPrefixExpression(PrefixExpression* prefix);

typedef struct {
	Expression base;
	Token token;
	Expression* left;
	char* op;
	Expression* right;
} InfixExpression;

InfixExpression* CreateInfixExpression(Token token, Expression* left, char* op, Expression* right);
char* InfixExpressionTokenLiteral(const InfixExpression* infix);
char* InfixExpressionString(const InfixExpression* infix);
void DestroyInfixExpression(InfixExpression* infix);

typedef struct {
	Statement base;
	Token token;
	Identifier* identifier;
	Expression* value;
} LetStatement;

LetStatement* CreateLetStatement(Token token, Identifier* identifier, Expression* value);
char* LetStatementTokenLiteral(const LetStatement* statement);
char* LetStatementString(const LetStatement* statement);

typedef struct {
	Statement base;
	Token token;
	Expression* returnValue;
} ReturnStatement;

ReturnStatement* CreateReturnStatement(Token token, Expression* returnValue);
char* ReturnStatementTokenLiteral(const ReturnStatement* statement);
char* ReturnStatementString(const ReturnStatement* statement);

typedef struct {
	Statement base;
	Token token;
	Expression* expression;
} ExpressionStatement;

ExpressionStatement* CreateExpressionStatement(Token token, Expression* expression);
char* ExpressionStatementTokenLiteral(const ExpressionStatement* statement);
char* ExpressionStatementString(const ExpressionStatement* statement);
