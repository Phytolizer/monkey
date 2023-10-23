#include "monkey/evaluator.h"

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/macros.h"
#include "monkey/object.h"
#include "monkey/string.h"
#include "span.h"

#include <assert.h>
#include <hedley.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	MonkeyInternedObjects interns;
	Environment* env;
} EvaluatorState;

MONKEY_FILE_LOCAL Object* evalStatement(EvaluatorState* state, Statement* statement);
MONKEY_FILE_LOCAL Object* evalExpression(EvaluatorState* state, Expression* expression);

MONKEY_FILE_LOCAL Object* HEDLEY_PRINTF_FORMAT(1, 2) newError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	char* message = MonkeyAvsprintf(format, args);
	va_end(args);

	return (Object*)CreateErrorObject(message);
}

MONKEY_FILE_LOCAL Object* evalProgram(EvaluatorState* state, Program* program) {
	Object* result = NULL;

	for (size_t i = 0; i < program->statements.length; i++) {
		DestroyObject(result);
		result = evalStatement(state, program->statements.begin[i]);
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

MONKEY_FILE_LOCAL Object* evalBlockStatement(EvaluatorState* state, BlockStatement* block) {
	Object* result = NULL;

	for (size_t i = 0; i < block->statements.length; i++) {
		DestroyObject(result);
		result = evalStatement(state, block->statements.begin[i]);
		if (result != NULL &&
				(result->type == OBJECT_TYPE_RETURN_VALUE || result->type == OBJECT_TYPE_ERROR)) {
			return result;
		}
	}

	return result;
}

MONKEY_FILE_LOCAL bool isTruthy(EvaluatorState* state, Object* value) {
	return !(value == state->interns.falseObj || value == state->interns.nullObj);
}

MONKEY_FILE_LOCAL bool isError(Object* value) {
	return value != NULL && value->type == OBJECT_TYPE_ERROR;
}

MONKEY_FILE_LOCAL Object* nativeBoolToBooleanObject(EvaluatorState* state, bool value) {
	return value ? state->interns.trueObj : state->interns.falseObj;
}

MONKEY_FILE_LOCAL ObjectSpan evalExpressions(EvaluatorState* state, ExpressionSpan exps) {
	Object** rawResult = calloc(exps.length, sizeof(Object*));

	for (size_t i = 0; i < exps.length; ++i) {
		Object* evaluated = evalExpression(state, exps.begin[i]);
		if (isError(evaluated)) {
			for (size_t j = 0; j < i; ++j) {
				DestroyObject(rawResult[i]);
			}
			free(rawResult);
			rawResult = malloc(1 * sizeof(Object*));
			rawResult[0] = evaluated;
			return (ObjectSpan)SPAN_WITH_LENGTH(rawResult, 1);
		}
		rawResult[i] = evaluated;
	}

	return (ObjectSpan)SPAN_WITH_LENGTH(rawResult, exps.length);
}

MONKEY_FILE_LOCAL Environment* extendFunctionEnv(FunctionObject* function, ObjectSpan arguments) {
	Environment* env = CreateEnvironment(function->env);

	for (size_t i = 0; i < function->parameters.length; ++i) {
		PutEnvironment(env, MonkeyStrdup(function->parameters.begin[i]->value), arguments.begin[i]);
	}

	return env;
}

MONKEY_FILE_LOCAL Object* unwrapReturnValue(Object* obj) {
	if (obj->type == OBJECT_TYPE_RETURN_VALUE) {
		ReturnValueObject* rv = (ReturnValueObject*)obj;
		Object* result = rv->value;
		rv->value = NULL;
		DestroyObject(&rv->base);
		return result;
	}
	return obj;
}

MONKEY_FILE_LOCAL Object* applyFunction(
		EvaluatorState* state, Object* functionObj, ObjectSpan arguments) {
	if (functionObj->type != OBJECT_TYPE_FUNCTION) {
		ObjectType funcType = functionObj->type;
		DestroyObject(functionObj);
		for (size_t i = 0; i < arguments.length; ++i) {
			DestroyObject(arguments.begin[i]);
		}
		free(arguments.begin);
		return newError("not a function: %s", ObjectTypeText(funcType));
	}
	FunctionObject* function = (FunctionObject*)functionObj;

	Environment* extendedEnv = extendFunctionEnv(function, arguments);
	Environment* oldEnvironment = state->env;
	state->env = extendedEnv;
	Object* result = evalBlockStatement(state, function->body);
	DestroyObject(functionObj);
	free(arguments.begin);
	DestroyEnvironment(extendedEnv);
	state->env = oldEnvironment;
	return unwrapReturnValue(result);
}

MONKEY_FILE_LOCAL Object* evalIdentifier(EvaluatorState* state, Identifier* identifier) {
	Object* val = GetEnvironment(state->env, identifier->value);
	if (val == NULL) {
		return newError("identifier not found: %s", identifier->value);
	}

	return CopyObject(val);
}

MONKEY_FILE_LOCAL Object* evalIfExpression(EvaluatorState* state, IfExpression* exp) {
	Object* condition = evalExpression(state, exp->condition);
	if (isError(condition)) {
		return condition;
	}
	bool truthy = isTruthy(state, condition);
	DestroyObject(condition);

	if (truthy) {
		return evalBlockStatement(state, exp->consequence);
	}
	if (exp->alternative != NULL) {
		return evalBlockStatement(state, exp->alternative);
	}
	return state->interns.nullObj;
}

MONKEY_FILE_LOCAL Object* evalBangOperatorExpression(EvaluatorState* state, Object* right) {
	return nativeBoolToBooleanObject(state, !isTruthy(state, right));
}

MONKEY_FILE_LOCAL Object* evalMinusPrefixOperatorExpression(Object* right) {
	if (right->type != OBJECT_TYPE_INTEGER) {
		return newError("unknown operator: -%s", ObjectTypeText(right->type));
	}

	int64_t value = ((IntegerObject*)right)->value;
	return (Object*)CreateIntegerObject(-value);
}

MONKEY_FILE_LOCAL Object* evalPrefixExpression(
		EvaluatorState* state, const char* op, Object* right) {
	if (strcmp(op, "!") == 0) {
		return evalBangOperatorExpression(state, right);
	}
	if (strcmp(op, "-") == 0) {
		return evalMinusPrefixOperatorExpression(right);
	}
	return newError("unknown operator: %s%s", op, ObjectTypeText(right->type));
}

MONKEY_FILE_LOCAL Object* evalIntegerInfixExpression(
		EvaluatorState* state, const char* op, IntegerObject* left, IntegerObject* right) {
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
		return nativeBoolToBooleanObject(state, left->value < right->value);
	}
	if (strcmp(op, ">") == 0) {
		return nativeBoolToBooleanObject(state, left->value > right->value);
	}
	if (strcmp(op, "==") == 0) {
		return nativeBoolToBooleanObject(state, left->value == right->value);
	}
	if (strcmp(op, "!=") == 0) {
		return nativeBoolToBooleanObject(state, left->value != right->value);
	}
	return newError("unknown operator: INTEGER %s INTEGER", op);
}

MONKEY_FILE_LOCAL Object* evalInfixExpression(
		EvaluatorState* state, const char* op, Object* left, Object* right) {
	if (left->type == OBJECT_TYPE_INTEGER && right->type == OBJECT_TYPE_INTEGER) {
		return evalIntegerInfixExpression(state, op, (IntegerObject*)left, (IntegerObject*)right);
	}
	if (strcmp(op, "==") == 0) {
		return nativeBoolToBooleanObject(state, left == right);
	}
	if (strcmp(op, "!=") == 0) {
		return nativeBoolToBooleanObject(state, left != right);
	}
	if (left->type != right->type) {
		return newError("type mismatch: %s %s %s", ObjectTypeText(left->type), op,
				ObjectTypeText(right->type));
	}
	return newError("unknown operator: %s %s %s", ObjectTypeText(left->type), op,
			ObjectTypeText(right->type));
}

MONKEY_FILE_LOCAL Object* evalStatement(EvaluatorState* state, Statement* statement) {
	switch (statement->type) {
		case STATEMENT_TYPE_EXPRESSION:
			return evalExpression(state, ((ExpressionStatement*)statement)->expression);
		case STATEMENT_TYPE_RETURN: {
			ReturnStatement* ret = (ReturnStatement*)statement;
			Object* val = evalExpression(state, ret->returnValue);
			if (isError(val)) {
				return val;
			}
			return (Object*)CreateReturnValueObject(val);
		}
		case STATEMENT_TYPE_LET: {
			LetStatement* let = (LetStatement*)statement;
			Object* val = evalExpression(state, let->value);
			if (isError(val)) {
				return val;
			}

			PutEnvironment(state->env, MonkeyStrdup(let->identifier->value), val);
			return state->interns.nullObj;
		}
		case STATEMENT_TYPE_BLOCK:
			return NULL;
	}
	(void)fprintf(stderr, "Unknown statement type: %d\n", statement->type);
	assert(false);
}

MONKEY_FILE_LOCAL Object* evalExpression(EvaluatorState* state, Expression* expression) {
	switch (expression->type) {
		case EXPRESSION_TYPE_INTEGER_LITERAL: {
			IntegerLiteral* lit = (IntegerLiteral*)expression;
			return (Object*)CreateIntegerObject(lit->value);
		}
		case EXPRESSION_TYPE_BOOLEAN_LITERAL: {
			BooleanLiteral* lit = (BooleanLiteral*)expression;
			return nativeBoolToBooleanObject(state, lit->value);
		}
		case EXPRESSION_TYPE_PREFIX: {
			PrefixExpression* prefix = (PrefixExpression*)expression;
			Object* right = evalExpression(state, prefix->right);
			if (isError(right)) {
				return right;
			}
			Object* result = evalPrefixExpression(state, prefix->op, right);

			DestroyObject(right);
			return result;
		}
		case EXPRESSION_TYPE_INFIX: {
			InfixExpression* infix = (InfixExpression*)expression;
			Object* left = evalExpression(state, infix->left);
			if (isError(left)) {
				return left;
			}
			Object* right = evalExpression(state, infix->right);
			if (isError(right)) {
				return right;
			}

			Object* result = evalInfixExpression(state, infix->op, left, right);
			DestroyObject(right);
			DestroyObject(left);
			return result;
		}
		case EXPRESSION_TYPE_IF:
			return evalIfExpression(state, (IfExpression*)expression);
		case EXPRESSION_TYPE_IDENTIFIER:
			return evalIdentifier(state, (Identifier*)expression);
		case EXPRESSION_TYPE_FUNCTION_LITERAL: {
			FunctionLiteral* func = (FunctionLiteral*)expression;
			return (Object*)CreateFunctionObject(func, CopyEnvironment(state->env));
		}
		case EXPRESSION_TYPE_CALL: {
			CallExpression* call = (CallExpression*)expression;
			Object* function = evalExpression(state, call->function);
			if (isError(function)) {
				return function;
			}
			ObjectSpan args = evalExpressions(state, call->arguments);
			if (args.length == 1 && isError(args.begin[0])) {
				Object* result = args.begin[0];
				free(args.begin);
				DestroyObject(function);
				return result;
			}
			return applyFunction(state, function, args);
		}
	}
	(void)fprintf(stderr, "Unknown expression type: %d\n", expression->type);
	assert(false);
}

Object* Eval(Monkey* monkey, Environment* env, Node* node) {
	EvaluatorState state = {
			.interns = MonkeyGetInterns(monkey),
			.env = env,
	};
	switch (node->type) {
		case NODE_TYPE_PROGRAM:
			return evalProgram(&state, (Program*)node);
		case NODE_TYPE_STATEMENT:
			return evalStatement(&state, (Statement*)node);
		case NODE_TYPE_EXPRESSION:
			return evalExpression(&state, (Expression*)node);
	}
	(void)fprintf(stderr, "Unknown node type: %d\n", node->type);
	assert(false);
}
