#include "monkey/ast.h"

#include "monkey/macros.h"
#include "monkey/string.h"

#include <assert.h>
#include <stdbool.h>
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

MONKEY_FILE_LOCAL void destroyExpression(Expression* expression) {
	free(expression);
}

MONKEY_FILE_LOCAL void destroyLetStatement(LetStatement* statement) {
	DestroyToken(&statement->token);
	DestroyIdentifier(statement->identifier);
	free(statement->identifier);
	destroyExpression(statement->value);
}

MONKEY_FILE_LOCAL void destroyReturnStatement(ReturnStatement* statement) {
	DestroyToken(&statement->token);
	destroyExpression(statement->returnValue);
}

MONKEY_FILE_LOCAL void destroyStatement(Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			destroyLetStatement((LetStatement*)statement);
			break;
		case STATEMENT_TYPE_RETURN:
			destroyReturnStatement((ReturnStatement*)statement);
			break;
		default:
			(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
			assert(false);
	}
	free(statement);
}

char* StatementTokenLiteral(const Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			return LetStatementTokenLiteral((const LetStatement*)statement);
		case STATEMENT_TYPE_RETURN:
			return ReturnStatementTokenLiteral((const ReturnStatement*)statement);
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
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

void DestroyIdentifier(Identifier* identifier) {
	DestroyToken(&identifier->token);
	free(identifier->value);
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
