#pragma once

#include "buffer.h"
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
	X(EXPRESSION) \
	X(BLOCK)

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
	X(INFIX) \
	X(IF) \
	X(FUNCTION_LITERAL) \
	X(CALL)

typedef enum {
#define X(name) EXPRESSION_TYPE_##name,
	EXPRESSION_TYPES_X
#undef X
} ExpressionType;

typedef struct {
	Node base;
	ExpressionType type;
} Expression;

typedef BUFFER_TYPE(Expression*) ExpressionBuffer;
typedef SPAN_TYPE(Expression*) ExpressionSpan;

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

typedef BUFFER_TYPE(Identifier*) IdentifierBuffer;
typedef SPAN_TYPE(Identifier*) IdentifierSpan;

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

struct BlockStatement;

typedef struct {
	Expression base;
	Token token;
	Expression* condition;
	struct BlockStatement* consequence;
	struct BlockStatement* alternative;
} IfExpression;

IfExpression* CreateIfExpression(Token token, Expression* condition,
		struct BlockStatement* consequence, struct BlockStatement* alternative);
char* IfExpressionTokenLiteral(const IfExpression* exp);
char* IfExpressionString(const IfExpression* exp);
void DestroyIfExpression(IfExpression* exp);

typedef struct {
	Expression base;
	Token token;
	IdentifierSpan parameters;
	struct BlockStatement* body;
} FunctionLiteral;

FunctionLiteral* CreateFunctionLiteral(
		Token token, IdentifierSpan parameters, struct BlockStatement* body);
char* FunctionLiteralTokenLiteral(const FunctionLiteral* exp);
char* FunctionLiteralString(const FunctionLiteral* exp);
void DestroyFunctionLiteral(FunctionLiteral* exp);

typedef struct {
	Expression base;
	Token token;
	Expression* function;
	ExpressionSpan arguments;
} CallExpression;

CallExpression* CreateCallExpression(Token token, Expression* function, ExpressionSpan arguments);
char* CallExpressionTokenLiteral(const CallExpression* exp);
char* CallExpressionString(const CallExpression* exp);
void DestroyCallExpression(CallExpression* exp);

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

typedef struct BlockStatement {
	Statement base;
	Token token;
	StatementSpan statements;
} BlockStatement;

BlockStatement* CreateBlockStatement(Token token, StatementSpan statements);
char* BlockStatementTokenLiteral(const BlockStatement* statement);
char* BlockStatementString(const BlockStatement* statement);
void DestroyBlockStatement(BlockStatement* statement);
