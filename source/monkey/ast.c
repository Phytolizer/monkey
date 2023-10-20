#include "monkey/ast.h"

#include "buffer.h"
#include "monkey/macros.h"
#include "monkey/string.h"
#include "monkey/token.h"
#include "span.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

MONKEY_FILE_LOCAL void initStatement(Statement* statement, StatementType type) {
	statement->base.type = NODE_TYPE_STATEMENT;
	statement->type = type;
}

MONKEY_FILE_LOCAL void initExpression(Expression* expression, ExpressionType type) {
	expression->base.type = NODE_TYPE_EXPRESSION;
	expression->type = type;
}

void DestroyExpression(Expression* expression) {
	if (expression == NULL) {
		return;
	}
	switch (expression->type) {
		case EXPRESSION_TYPE_IDENTIFIER:
			DestroyIdentifier((Identifier*)expression);
			return;
		case EXPRESSION_TYPE_INTEGER_LITERAL:
			DestroyIntegerLiteral((IntegerLiteral*)expression);
			return;
		case EXPRESSION_TYPE_BOOLEAN_LITERAL:
			DestroyBooleanLiteral((BooleanLiteral*)expression);
			return;
		case EXPRESSION_TYPE_PREFIX:
			DestroyPrefixExpression((PrefixExpression*)expression);
			return;
		case EXPRESSION_TYPE_INFIX:
			DestroyInfixExpression((InfixExpression*)expression);
			return;
		case EXPRESSION_TYPE_IF:
			DestroyIfExpression((IfExpression*)expression);
			return;
		case EXPRESSION_TYPE_FUNCTION_LITERAL:
			DestroyFunctionLiteral((FunctionLiteral*)expression);
			return;
		case EXPRESSION_TYPE_CALL:
			DestroyCallExpression((CallExpression*)expression);
			return;
	}
	(void)fprintf(stderr, "Unknown expression type: %d\n", expression->type);
	assert(false);
}

MONKEY_FILE_LOCAL void destroyStatement(Statement* statement);

MONKEY_FILE_LOCAL void destroyLetStatement(LetStatement* statement) {
	DestroyToken(&statement->token);
	DestroyIdentifier(statement->identifier);
	DestroyExpression(statement->value);
	free(statement);
}

MONKEY_FILE_LOCAL void destroyReturnStatement(ReturnStatement* statement) {
	DestroyToken(&statement->token);
	DestroyExpression(statement->returnValue);
	free(statement);
}

MONKEY_FILE_LOCAL void destroyExpressionStatement(ExpressionStatement* statement) {
	DestroyToken(&statement->token);
	DestroyExpression(statement->expression);
	free(statement);
}

MONKEY_FILE_LOCAL void destroyStatement(Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			destroyLetStatement((LetStatement*)statement);
			return;
		case STATEMENT_TYPE_RETURN:
			destroyReturnStatement((ReturnStatement*)statement);
			return;
		case STATEMENT_TYPE_EXPRESSION:
			destroyExpressionStatement((ExpressionStatement*)statement);
			return;
		case STATEMENT_TYPE_BLOCK:
			DestroyBlockStatement((BlockStatement*)statement);
			return;
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
}

char* StatementTokenLiteral(const Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			return LetStatementTokenLiteral((const LetStatement*)statement);
		case STATEMENT_TYPE_RETURN:
			return ReturnStatementTokenLiteral((const ReturnStatement*)statement);
		case STATEMENT_TYPE_EXPRESSION:
			return ExpressionStatementTokenLiteral((const ExpressionStatement*)statement);
		case STATEMENT_TYPE_BLOCK:
			return BlockStatementTokenLiteral((const BlockStatement*)statement);
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
	return NULL;
}

char* StatementString(const Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			return LetStatementString((const LetStatement*)statement);
		case STATEMENT_TYPE_RETURN:
			return ReturnStatementString((const ReturnStatement*)statement);
		case STATEMENT_TYPE_EXPRESSION:
			return ExpressionStatementString((const ExpressionStatement*)statement);
		case STATEMENT_TYPE_BLOCK:
			return BlockStatementString((const BlockStatement*)statement);
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
	return NULL;
}

char* ExpressionString(const Expression* expression) {
	switch (expression->type) {
		case EXPRESSION_TYPE_IDENTIFIER:
			return IdentifierString((const Identifier*)expression);
		case EXPRESSION_TYPE_INTEGER_LITERAL:
			return IntegerLiteralString((const IntegerLiteral*)expression);
		case EXPRESSION_TYPE_BOOLEAN_LITERAL:
			return BooleanLiteralString((const BooleanLiteral*)expression);
		case EXPRESSION_TYPE_PREFIX:
			return PrefixExpressionString((const PrefixExpression*)expression);
		case EXPRESSION_TYPE_INFIX:
			return InfixExpressionString((const InfixExpression*)expression);
		case EXPRESSION_TYPE_IF:
			return IfExpressionString((const IfExpression*)expression);
		case EXPRESSION_TYPE_FUNCTION_LITERAL:
			return FunctionLiteralString((const FunctionLiteral*)expression);
		case EXPRESSION_TYPE_CALL:
			return CallExpressionString((const CallExpression*)expression);
	}
	(void)fprintf(stderr, "Unknown expression type: %d\n", expression->type);
	assert(false);
	return NULL;
}

Program* CreateProgram(StatementSpan statements) {
	Program* program = calloc(1, sizeof(Program));
	program->base.type = NODE_TYPE_PROGRAM;
	program->statements = statements;
	return program;
}

char* ProgramTokenLiteral(const Program* program) {
	if (program->statements.length > 0) {
		return StatementTokenLiteral(program->statements.begin[0]);
	}

	return MonkeyStrdup("");
}

char* ProgramString(const Program* program) {
	MonkeyStringBuffer statementStrings = BUFFER_INIT;
	for (size_t i = 0; i < program->statements.length; ++i) {
		BUFFER_PUSH(&statementStrings, StatementString(program->statements.begin[i]));
	}
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(statementStrings));
	for (size_t i = 0; i < statementStrings.length; ++i) {
		free(statementStrings.data[i]);
	}
	BUFFER_FREE(statementStrings);
	return result;
}

void DestroyProgram(Program* program) {
	for (size_t i = 0; i < program->statements.length; i++) {
		destroyStatement(program->statements.begin[i]);
	}
	free(program->statements.begin);
	free(program);
}

Identifier* CreateIdentifier(Token token, char* value) {
	Identifier* identifier = calloc(1, sizeof(Identifier));
	initExpression(&identifier->base, EXPRESSION_TYPE_IDENTIFIER);
	identifier->token = token;
	identifier->value = value;
	return identifier;
}

char* IdentifierTokenLiteral(const Identifier* identifier) {
	return MonkeyStrdup(identifier->token.literal);
}

char* IdentifierString(const Identifier* identifier) {
	return MonkeyStrdup(identifier->value);
}

void DestroyIdentifier(Identifier* identifier) {
	DestroyToken(&identifier->token);
	free(identifier->value);
	free(identifier);
}

IntegerLiteral* CreateIntegerLiteral(Token token, int64_t value) {
	IntegerLiteral* integerLiteral = calloc(1, sizeof(IntegerLiteral));
	initExpression(&integerLiteral->base, EXPRESSION_TYPE_INTEGER_LITERAL);
	integerLiteral->token = token;
	integerLiteral->value = value;
	return integerLiteral;
}

char* IntegerLiteralTokenLiteral(const IntegerLiteral* integerLiteral) {
	return MonkeyStrdup(integerLiteral->token.literal);
}

char* IntegerLiteralString(const IntegerLiteral* integerLiteral) {
	return MonkeyStrdup(integerLiteral->token.literal);
}

void DestroyIntegerLiteral(IntegerLiteral* integerLiteral) {
	DestroyToken(&integerLiteral->token);
	free(integerLiteral);
}

BooleanLiteral* CreateBooleanLiteral(Token token, int64_t value) {
	BooleanLiteral* booleanLiteral = calloc(1, sizeof(BooleanLiteral));
	initExpression(&booleanLiteral->base, EXPRESSION_TYPE_BOOLEAN_LITERAL);
	booleanLiteral->token = token;
	booleanLiteral->value = value;
	return booleanLiteral;
}

char* BooleanLiteralTokenLiteral(const BooleanLiteral* booleanLiteral) {
	return MonkeyStrdup(booleanLiteral->token.literal);
}

char* BooleanLiteralString(const BooleanLiteral* booleanLiteral) {
	return MonkeyStrdup(booleanLiteral->token.literal);
}

void DestroyBooleanLiteral(BooleanLiteral* booleanLiteral) {
	DestroyToken(&booleanLiteral->token);
	free(booleanLiteral);
}

PrefixExpression* CreatePrefixExpression(Token token, char* op, Expression* right) {
	PrefixExpression* prefix = calloc(1, sizeof(PrefixExpression));
	initExpression(&prefix->base, EXPRESSION_TYPE_PREFIX);
	prefix->token = token;
	prefix->op = op;
	prefix->right = right;
	return prefix;
}

char* PrefixExpressionTokenLiteral(const PrefixExpression* prefix) {
	return MonkeyStrdup(prefix->token.literal);
}

char* PrefixExpressionString(const PrefixExpression* prefix) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, MonkeyStrdup("("));
	BUFFER_PUSH(&out, MonkeyStrdup(prefix->op));
	BUFFER_PUSH(&out, ExpressionString(prefix->right));
	BUFFER_PUSH(&out, MonkeyStrdup(")"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyPrefixExpression(PrefixExpression* prefix) {
	DestroyToken(&prefix->token);
	free(prefix->op);
	DestroyExpression(prefix->right);
	free(prefix);
}

InfixExpression* CreateInfixExpression(Token token, Expression* left, char* op, Expression* right) {
	InfixExpression* infix = calloc(1, sizeof(InfixExpression));
	initExpression(&infix->base, EXPRESSION_TYPE_INFIX);
	infix->token = token;
	infix->left = left;
	infix->op = op;
	infix->right = right;
	return infix;
}

char* InfixExpressionTokenLiteral(const InfixExpression* infix) {
	return MonkeyStrdup(infix->token.literal);
}

char* InfixExpressionString(const InfixExpression* infix) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, MonkeyStrdup("("));
	BUFFER_PUSH(&out, ExpressionString(infix->left));
	BUFFER_PUSH(&out, MonkeyStrdup(" "));
	BUFFER_PUSH(&out, MonkeyStrdup(infix->op));
	BUFFER_PUSH(&out, MonkeyStrdup(" "));
	BUFFER_PUSH(&out, ExpressionString(infix->right));
	BUFFER_PUSH(&out, MonkeyStrdup(")"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyInfixExpression(InfixExpression* infix) {
	DestroyToken(&infix->token);
	DestroyExpression(infix->left);
	free(infix->op);
	DestroyExpression(infix->right);
	free(infix);
}

IfExpression* CreateIfExpression(Token token, Expression* condition, BlockStatement* consequence,
		BlockStatement* alternative) {
	IfExpression* exp = calloc(1, sizeof(IfExpression));
	initExpression(&exp->base, EXPRESSION_TYPE_IF);
	exp->token = token;
	exp->condition = condition;
	exp->consequence = consequence;
	exp->alternative = alternative;
	return exp;
}

char* IfExpressionTokenLiteral(const IfExpression* exp) {
	return MonkeyStrdup(exp->token.literal);
}

char* IfExpressionString(const IfExpression* exp) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, MonkeyStrdup("if"));
	BUFFER_PUSH(&out, ExpressionString(exp->condition));
	BUFFER_PUSH(&out, MonkeyStrdup(" "));
	BUFFER_PUSH(&out, StatementString(&exp->consequence->base));
	if (exp->alternative != NULL) {
		BUFFER_PUSH(&out, MonkeyStrdup(" else "));
		BUFFER_PUSH(&out, StatementString(&exp->alternative->base));
	}
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyIfExpression(IfExpression* exp) {
	DestroyToken(&exp->token);
	DestroyExpression(exp->condition);
	DestroyBlockStatement(exp->consequence);
	DestroyBlockStatement(exp->alternative);
	free(exp);
}

FunctionLiteral* CreateFunctionLiteral(
		Token token, IdentifierSpan parameters, BlockStatement* body) {
	FunctionLiteral* exp = calloc(1, sizeof(FunctionLiteral));
	initExpression(&exp->base, EXPRESSION_TYPE_FUNCTION_LITERAL);
	exp->token = token;
	exp->parameters = parameters;
	exp->body = body;
	return exp;
}

char* FunctionLiteralTokenLiteral(const FunctionLiteral* exp) {
	return MonkeyStrdup(exp->token.literal);
}

char* FunctionLiteralString(const FunctionLiteral* exp) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, FunctionLiteralTokenLiteral(exp));
	BUFFER_PUSH(&out, MonkeyStrdup("("));
	for (size_t i = 0; i < exp->parameters.length; ++i) {
		if (i > 0) {
			BUFFER_PUSH(&out, MonkeyStrdup(", "));
		}
		BUFFER_PUSH(&out, IdentifierString(exp->parameters.begin[i]));
	}
	BUFFER_PUSH(&out, MonkeyStrdup(")"));
	BUFFER_PUSH(&out, BlockStatementString(exp->body));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyFunctionLiteral(FunctionLiteral* exp) {
	DestroyToken(&exp->token);
	for (size_t i = 0; i < exp->parameters.length; ++i) {
		DestroyIdentifier(exp->parameters.begin[i]);
	}
	free(exp->parameters.begin);
	DestroyBlockStatement(exp->body);
	free(exp);
}

CallExpression* CreateCallExpression(Token token, Expression* function, ExpressionSpan arguments) {
	CallExpression* exp = calloc(1, sizeof(CallExpression));
	initExpression(&exp->base, EXPRESSION_TYPE_CALL);
	exp->token = token;
	exp->function = function;
	exp->arguments = arguments;
	return exp;
}

char* CallExpressionTokenLiteral(const CallExpression* exp) {
	return MonkeyStrdup(exp->token.literal);
}

char* CallExpressionString(const CallExpression* exp) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, ExpressionString(exp->function));
	BUFFER_PUSH(&out, MonkeyStrdup("("));
	for (size_t i = 0; i < exp->arguments.length; ++i) {
		if (i > 0) {
			BUFFER_PUSH(&out, MonkeyStrdup(", "));
		}
		BUFFER_PUSH(&out, ExpressionString(exp->arguments.begin[i]));
	}
	BUFFER_PUSH(&out, MonkeyStrdup(")"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyCallExpression(CallExpression* exp) {
	DestroyToken(&exp->token);
	DestroyExpression(exp->function);
	for (size_t i = 0; i < exp->arguments.length; ++i) {
		DestroyExpression(exp->arguments.begin[i]);
	}
	free(exp->arguments.begin);
	free(exp);
}

LetStatement* CreateLetStatement(Token token, Identifier* identifier, Expression* value) {
	LetStatement* statement = calloc(1, sizeof(LetStatement));
	initStatement(&statement->base, STATEMENT_TYPE_LET);
	statement->token = token;
	statement->identifier = identifier;
	statement->value = value;
	return statement;
}

char* LetStatementTokenLiteral(const LetStatement* statement) {
	return MonkeyStrdup(statement->token.literal);
}

char* LetStatementString(const LetStatement* statement) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, LetStatementTokenLiteral(statement));
	BUFFER_PUSH(&out, MonkeyStrdup(" "));
	BUFFER_PUSH(&out, IdentifierString(statement->identifier));
	BUFFER_PUSH(&out, MonkeyStrdup(" = "));
	if (statement->value != NULL) {
		BUFFER_PUSH(&out, ExpressionString(statement->value));
	}
	BUFFER_PUSH(&out, MonkeyStrdup(";"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

ReturnStatement* CreateReturnStatement(Token token, Expression* returnValue) {
	ReturnStatement* statement = calloc(1, sizeof(ReturnStatement));
	initStatement(&statement->base, STATEMENT_TYPE_RETURN);
	statement->token = token;
	statement->returnValue = returnValue;
	return statement;
}

char* ReturnStatementTokenLiteral(const ReturnStatement* statement) {
	return MonkeyStrdup(statement->token.literal);
}

char* ReturnStatementString(const ReturnStatement* statement) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, ReturnStatementTokenLiteral(statement));
	BUFFER_PUSH(&out, MonkeyStrdup(" "));
	if (statement->returnValue != NULL) {
		BUFFER_PUSH(&out, ExpressionString(statement->returnValue));
	}
	BUFFER_PUSH(&out, MonkeyStrdup(";"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

ExpressionStatement* CreateExpressionStatement(Token token, Expression* expression) {
	ExpressionStatement* statement = calloc(1, sizeof(ExpressionStatement));
	initStatement(&statement->base, STATEMENT_TYPE_EXPRESSION);
	statement->token = token;
	statement->expression = expression;
	return statement;
}

char* ExpressionStatementTokenLiteral(const ExpressionStatement* statement) {
	return MonkeyStrdup(statement->token.literal);
}

char* ExpressionStatementString(const ExpressionStatement* statement) {
	if (statement->expression) {
		return ExpressionString(statement->expression);
	}
	return MonkeyStrdup("");
}

BlockStatement* CreateBlockStatement(Token token, StatementSpan statements) {
	BlockStatement* statement = calloc(1, sizeof(BlockStatement));
	initStatement(&statement->base, STATEMENT_TYPE_BLOCK);
	statement->token = token;
	statement->statements = statements;
	return statement;
}

char* BlockStatementTokenLiteral(const BlockStatement* statement) {
	return MonkeyStrdup(statement->token.literal);
}

char* BlockStatementString(const BlockStatement* statement) {
	MonkeyStringBuffer out = BUFFER_INIT;
	for (size_t i = 0; i < statement->statements.length; ++i) {
		BUFFER_PUSH(&out, StatementString(statement->statements.begin[i]));
	}
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyBlockStatement(BlockStatement* statement) {
	if (statement == NULL) {
		return;
	}
	DestroyToken(&statement->token);
	for (size_t i = 0; i < statement->statements.length; ++i) {
		destroyStatement(statement->statements.begin[i]);
	}
	free(statement->statements.begin);
	free(statement);
}
