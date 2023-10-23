#include "monkey/evaluator.h"

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/macros.h"
#include "monkey/object.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

MONKEY_FILE_LOCAL Object* evalProgram(Monkey* monkey, Program* program) {
	Object* result = NULL;

	for (size_t i = 0; i < program->statements.length; i++) {
		DestroyObject(result);
		result = Eval(monkey, &program->statements.begin[i]->base);
	}

	return result;
}

MONKEY_FILE_LOCAL Object* evalBangOperatorExpression(Monkey* monkey, Object* right) {
	MonkeyInternedObjects interns = MonkeyGetInterns(monkey);
	if (right == interns.falseObj || right == interns.nullObj) {
		return interns.trueObj;
	}
	return interns.falseObj;
}

MONKEY_FILE_LOCAL Object* evalMinusPrefixOperatorExpression(Monkey* monkey, Object* right) {
	if (right->type != OBJECT_TYPE_INTEGER) {
		return MonkeyGetInterns(monkey).nullObj;
	}

	int64_t value = ((IntegerObject*)right)->value;
	return (Object*)CreateIntegerObject(-value);
}

MONKEY_FILE_LOCAL Object* evalPrefixExpression(Monkey* monkey, const char* op, Object* right) {
	if (strcmp(op, "!") == 0) {
		return evalBangOperatorExpression(monkey, right);
	}
	if (strcmp(op, "-") == 0) {
		return evalMinusPrefixOperatorExpression(monkey, right);
	}
	return MonkeyGetInterns(monkey).nullObj;
}

MONKEY_FILE_LOCAL Object* nativeBoolToBooleanObject(Monkey* monkey, bool value) {
	MonkeyInternedObjects interns = MonkeyGetInterns(monkey);
	return value ? interns.trueObj : interns.falseObj;
}

MONKEY_FILE_LOCAL Object* evalIntegerInfixExpression(
		Monkey* monkey, const char* op, IntegerObject* left, IntegerObject* right) {
	if (strcmp(op, "+") == 0) {
		return (Object*)CreateIntegerObject(left->value + right->value);
	}
	if (strcmp(op, "-") == 0) {
		return (Object*)CreateIntegerObject(left->value - right->value);
	}
	if (strcmp(op, "*") == 0) {
		return (Object*)CreateIntegerObject(left->value * right->value);
	}
	if (strcmp(op, "/") == 0) {
		return (Object*)CreateIntegerObject(left->value / right->value);
	}
	if (strcmp(op, "<") == 0) {
		return nativeBoolToBooleanObject(monkey, left->value < right->value);
	}
	if (strcmp(op, ">") == 0) {
		return nativeBoolToBooleanObject(monkey, left->value > right->value);
	}
	if (strcmp(op, "==") == 0) {
		return nativeBoolToBooleanObject(monkey, left->value == right->value);
	}
	if (strcmp(op, "!=") == 0) {
		return nativeBoolToBooleanObject(monkey, left->value != right->value);
	}
	return MonkeyGetInterns(monkey).nullObj;
}

MONKEY_FILE_LOCAL Object* evalInfixExpression(
		Monkey* monkey, const char* op, Object* left, Object* right) {
	if (left->type == OBJECT_TYPE_INTEGER && right->type == OBJECT_TYPE_INTEGER) {
		return evalIntegerInfixExpression(monkey, op, (IntegerObject*)left, (IntegerObject*)right);
	}
	return MonkeyGetInterns(monkey).nullObj;
}

MONKEY_FILE_LOCAL Object* evalStatement(Monkey* monkey, Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_EXPRESSION:
			return Eval(monkey, &((ExpressionStatement*)statement)->expression->base);
		case STATEMENT_TYPE_LET:
		case STATEMENT_TYPE_RETURN:
		case STATEMENT_TYPE_BLOCK:
			return NULL;
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
}

MONKEY_FILE_LOCAL Object* evalExpression(Monkey* monkey, Expression* expression) {
	switch (expression->type) {
		case EXPRESSION_TYPE_INTEGER_LITERAL: {
			IntegerLiteral* lit = (IntegerLiteral*)expression;
			return (Object*)CreateIntegerObject(lit->value);
		}
		case EXPRESSION_TYPE_BOOLEAN_LITERAL: {
			BooleanLiteral* lit = (BooleanLiteral*)expression;
			return nativeBoolToBooleanObject(monkey, lit->value);
		}
		case EXPRESSION_TYPE_PREFIX: {
			PrefixExpression* prefix = (PrefixExpression*)expression;
			Object* right = evalExpression(monkey, prefix->right);
			Object* result = evalPrefixExpression(monkey, prefix->op, right);

			DestroyObject(right);
			return result;
		}
		case EXPRESSION_TYPE_INFIX: {
			InfixExpression* infix = (InfixExpression*)expression;
			Object* left = evalExpression(monkey, infix->left);
			Object* right = evalExpression(monkey, infix->right);

			Object* result = evalInfixExpression(monkey, infix->op, left, right);
			DestroyObject(right);
			DestroyObject(left);
			return result;
		}
		case EXPRESSION_TYPE_IDENTIFIER:
		case EXPRESSION_TYPE_IF:
		case EXPRESSION_TYPE_FUNCTION_LITERAL:
		case EXPRESSION_TYPE_CALL:
			return NULL;
	}
	(void)fprintf(stderr, "Unknown expression type: %d\n", expression->type);
	assert(false);
}

Object* Eval(Monkey* monkey, Node* node) {
	switch (node->type) {
		case NODE_TYPE_PROGRAM:
			return evalProgram(monkey, (Program*)node);
		case NODE_TYPE_STATEMENT:
			return evalStatement(monkey, (Statement*)node);
		case NODE_TYPE_EXPRESSION:
			return evalExpression(monkey, (Expression*)node);
	}
	(void)fprintf(stderr, "Unknown node type: %d\n", node->type);
	assert(false);
}
