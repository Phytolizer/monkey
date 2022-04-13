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

MONKEY_FILE_LOCAL void destroyIdentifier(Identifier* identifier) {
	free(identifier->value);
}

MONKEY_FILE_LOCAL void destroyLetStatement(LetStatement* statement) {
	DestroyToken(&statement->token);
	destroyIdentifier(statement->identifier);
	free(statement->value);
}

MONKEY_FILE_LOCAL void destroyStatement(Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_LET:
			destroyLetStatement((LetStatement*)statement);
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
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
	return NULL;
}

void InitProgram(Program* program, StatementSpan statements) {
	program->base.type = NODE_TYPE_PROGRAM;
	program->statements = statements;
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
}

void InitIdentifier(Identifier* identifier, Token token, char* value) {
	initExpression(&identifier->base, EXPRESSION_TYPE_IDENTIFIER);
	identifier->token = token;
	identifier->value = value;
}

char* IdentifierTokenLiteral(const Identifier* identifier) {
	return MonkeyStrdup(identifier->token.literal);
}

void InitLetStatement(
		LetStatement* statement, Token token, Identifier* identifier, Expression* value) {
	initStatement(&statement->base, STATEMENT_TYPE_LET);
	statement->token = token;
	statement->identifier = identifier;
	statement->value = value;
}

char* LetStatementTokenLiteral(const LetStatement* statement) {
	return MonkeyStrdup(statement->token.literal);
}
