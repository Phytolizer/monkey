#include "monkey/evaluator.h"

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/macros.h"
#include "monkey/object.h"
#include "monkey/string.h"

#include <assert.h>
#include <hedley.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

MONKEY_FILE_LOCAL Object* evalExpression(Monkey* monkey, Expression* expression);

MONKEY_FILE_LOCAL Object* HEDLEY_PRINTF_FORMAT(1, 2) newError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	char* message = MonkeyAvsprintf(format, args);
	va_end(args);

	return (Object*)CreateErrorObject(message);
}

MONKEY_FILE_LOCAL Object* evalProgram(Monkey* monkey, Program* program) {
	Object* result = NULL;

	for (size_t i = 0; i < program->statements.length; i++) {
		DestroyObject(result);
		result = Eval(monkey, &program->statements.begin[i]->base);
		if (result != NULL && result->type == OBJECT_TYPE_RETURN_VALUE) {
			ReturnValueObject* toFree = (ReturnValueObject*)result;
			result = toFree->value;
			toFree->value = NULL;
			DestroyObject(&toFree->base);
			return result;
		}
		if (result != NULL && result->type == OBJECT_TYPE_ERROR) {
			return result;
		}
	}

	return result;
}

MONKEY_FILE_LOCAL Object* evalBlockStatement(Monkey* monkey, BlockStatement* block) {
	Object* result = NULL;

	for (size_t i = 0; i < block->statements.length; i++) {
		DestroyObject(result);
		result = Eval(monkey, &block->statements.begin[i]->base);
		if (result != NULL &&
				(result->type == OBJECT_TYPE_RETURN_VALUE || result->type == OBJECT_TYPE_ERROR)) {
			return result;
		}
	}

	return result;
}

MONKEY_FILE_LOCAL bool isTruthy(Monkey* monkey, Object* value) {
	MonkeyInternedObjects interns = MonkeyGetInterns(monkey);
	return !(value == interns.falseObj || value == interns.nullObj);
}

MONKEY_FILE_LOCAL bool isError(Object* value) {
	return value != NULL && value->type == OBJECT_TYPE_ERROR;
}

MONKEY_FILE_LOCAL Object* nativeBoolToBooleanObject(Monkey* monkey, bool value) {
	MonkeyInternedObjects interns = MonkeyGetInterns(monkey);
	return value ? interns.trueObj : interns.falseObj;
}

MONKEY_FILE_LOCAL Object* evalIfExpression(Monkey* monkey, IfExpression* exp) {
	Object* condition = evalExpression(monkey, exp->condition);
	if (isError(condition)) {
		return condition;
	}
	bool truthy = isTruthy(monkey, condition);
	DestroyObject(condition);

	if (truthy) {
		return evalBlockStatement(monkey, exp->consequence);
	}
	if (exp->alternative != NULL) {
		return evalBlockStatement(monkey, exp->alternative);
	}
	return MonkeyGetInterns(monkey).nullObj;
}

MONKEY_FILE_LOCAL Object* evalBangOperatorExpression(Monkey* monkey, Object* right) {
	return nativeBoolToBooleanObject(monkey, !isTruthy(monkey, right));
}

MONKEY_FILE_LOCAL Object* evalMinusPrefixOperatorExpression(Object* right) {
	if (right->type != OBJECT_TYPE_INTEGER) {
		return newError("unknown operator: -%s", ObjectTypeText(right->type));
	}

	int64_t value = ((IntegerObject*)right)->value;
	return (Object*)CreateIntegerObject(-value);
}

MONKEY_FILE_LOCAL Object* evalPrefixExpression(Monkey* monkey, const char* op, Object* right) {
	if (strcmp(op, "!") == 0) {
		return evalBangOperatorExpression(monkey, right);
	}
	if (strcmp(op, "-") == 0) {
		return evalMinusPrefixOperatorExpression(right);
	}
	return newError("unknown operator: %s%s", op, ObjectTypeText(right->type));
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
	return newError("unknown operator: INTEGER %s INTEGER", op);
}

MONKEY_FILE_LOCAL Object* evalInfixExpression(
		Monkey* monkey, const char* op, Object* left, Object* right) {
	if (left->type == OBJECT_TYPE_INTEGER && right->type == OBJECT_TYPE_INTEGER) {
		return evalIntegerInfixExpression(monkey, op, (IntegerObject*)left, (IntegerObject*)right);
	}
	if (strcmp(op, "==") == 0) {
		return nativeBoolToBooleanObject(monkey, left == right);
	}
	if (strcmp(op, "!=") == 0) {
		return nativeBoolToBooleanObject(monkey, left != right);
	}
	if (left->type != right->type) {
		return newError("type mismatch: %s %s %s", ObjectTypeText(left->type), op,
				ObjectTypeText(right->type));
	}
	return newError("unknown operator: %s %s %s", ObjectTypeText(left->type), op,
			ObjectTypeText(right->type));
}

MONKEY_FILE_LOCAL Object* evalStatement(Monkey* monkey, Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_EXPRESSION:
			return Eval(monkey, &((ExpressionStatement*)statement)->expression->base);
		case STATEMENT_TYPE_RETURN: {
			ReturnStatement* ret = (ReturnStatement*)statement;
			Object* val = evalExpression(monkey, ret->returnValue);
			if (isError(val)) {
				return val;
			}
			return (Object*)CreateReturnValueObject(val);
		}
		case STATEMENT_TYPE_LET:
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
			if (isError(right)) {
				return right;
			}
			Object* result = evalPrefixExpression(monkey, prefix->op, right);

			DestroyObject(right);
			return result;
		}
		case EXPRESSION_TYPE_INFIX: {
			InfixExpression* infix = (InfixExpression*)expression;
			Object* left = evalExpression(monkey, infix->left);
			if (isError(left)) {
				return left;
			}
			Object* right = evalExpression(monkey, infix->right);
			if (isError(right)) {
				return right;
			}

			Object* result = evalInfixExpression(monkey, infix->op, left, right);
			DestroyObject(right);
			DestroyObject(left);
			return result;
		}
		case EXPRESSION_TYPE_IF:
			return evalIfExpression(monkey, (IfExpression*)expression);
		case EXPRESSION_TYPE_IDENTIFIER:
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
