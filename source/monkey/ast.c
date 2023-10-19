#include "monkey/ast.h"

#include "monkey/macros.h"
#include "monkey/string.h"
#include "monkey/token.h"
#include "monkey/vector.h"
#include "span.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
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
	if (expression == NULL) {
		return;
	}
	switch (expression->type) {
		case EXPRESSION_TYPE_IDENTIFIER:
			DestroyIdentifier((Identifier*)expression);
			return;
	}
	(void)fprintf(stderr, "Unknown expression type: %d\n", expression->type);
	assert(false);
}

MONKEY_FILE_LOCAL void destroyLetStatement(LetStatement* statement) {
	DestroyToken(&statement->token);
	DestroyIdentifier(statement->identifier);
	destroyExpression(statement->value);
	free(statement);
}

MONKEY_FILE_LOCAL void destroyReturnStatement(ReturnStatement* statement) {
	DestroyToken(&statement->token);
	destroyExpression(statement->returnValue);
	free(statement);
}

MONKEY_FILE_LOCAL void destroyExpressionStatement(ExpressionStatement* statement) {
	DestroyToken(&statement->token);
	destroyExpression(statement->expression);
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
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
	return NULL;
}

char* ExpressionString(const Expression* expression) {
	switch (expression->type) {
		case EXPRESSION_TYPE_IDENTIFIER:
			return IdentifierString((const Identifier*)expression);
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
	MonkeyStringVector statementStrings = VECTOR_INIT;
	for (size_t i = 0; i < program->statements.length; ++i) {
		VECTOR_PUSH(&statementStrings, StatementString(program->statements.begin[i]));
	}
	char* result = MonkeyStringJoin(
			(MonkeyStringSpan)SPAN_WITH_LENGTH(statementStrings.data, statementStrings.size));
	for (size_t i = 0; i < statementStrings.size; ++i) {
		free(statementStrings.data[i]);
	}
	VECTOR_FREE(&statementStrings);
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
	MonkeyStringVector out = VECTOR_INIT;
	VECTOR_PUSH(&out, LetStatementTokenLiteral(statement));
	VECTOR_PUSH(&out, MonkeyStrdup(" "));
	VECTOR_PUSH(&out, IdentifierString(statement->identifier));
	VECTOR_PUSH(&out, MonkeyStrdup(" = "));
	if (statement->value != NULL) {
		VECTOR_PUSH(&out, ExpressionString(statement->value));
	}
	VECTOR_PUSH(&out, MonkeyStrdup(";"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)SPAN_WITH_LENGTH(out.data, out.size));
	for (size_t i = 0; i < out.size; ++i) {
		free(out.data[i]);
	}
	VECTOR_FREE(&out);
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
	MonkeyStringVector out = VECTOR_INIT;
	VECTOR_PUSH(&out, ReturnStatementTokenLiteral(statement));
	VECTOR_PUSH(&out, MonkeyStrdup(" "));
	if (statement->returnValue != NULL) {
		VECTOR_PUSH(&out, ExpressionString(statement->returnValue));
	}
	VECTOR_PUSH(&out, MonkeyStrdup(";"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)SPAN_WITH_LENGTH(out.data, out.size));
	for (size_t i = 0; i < out.size; ++i) {
		free(out.data[i]);
	}
	VECTOR_FREE(&out);
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
