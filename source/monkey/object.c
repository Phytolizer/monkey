#include "monkey/object.h"

#include "buffer.h"
#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/macros.h"
#include "monkey/string.h"
#include "span.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char* ObjectTypeText(ObjectType type) {
	switch (type) {
#define X(name) \
	case OBJECT_TYPE_##name: \
		return #name;
		OBJECT_TYPES_X
#undef X
	}
	(void)fprintf(stderr, "Unknown object type: %d\n", type);
	assert(false);
	return NULL;
}

char* InspectObject(const Object* obj) {
	if (obj == NULL) {
		return MonkeyStrdup("<NULL>");
	}
	switch (obj->type) {
		case OBJECT_TYPE_INTEGER:
			return InspectIntegerObject((const IntegerObject*)obj);
		case OBJECT_TYPE_BOOLEAN:
			return InspectBooleanObject((const BooleanObject*)obj);
		case OBJECT_TYPE_NULL:
			return InspectNullObject((const NullObject*)obj);
		case OBJECT_TYPE_RETURN_VALUE:
			return InspectReturnValueObject((const ReturnValueObject*)obj);
		case OBJECT_TYPE_ERROR:
			return InspectErrorObject((const ErrorObject*)obj);
		case OBJECT_TYPE_FUNCTION:
			return InspectFunctionObject((const FunctionObject*)obj);
	}
	(void)fprintf(stderr, "Unknown object type: %d\n", obj->type);
	assert(false);
}

void DestroyObject(Object* obj) {
	if (obj == NULL) {
		return;
	}
	if (obj->freeable == OBJECT_DISALLOW_FREE) {
		return;
	}
	switch (obj->type) {
		case OBJECT_TYPE_INTEGER:
			DestroyIntegerObject((IntegerObject*)obj);
			return;
		case OBJECT_TYPE_BOOLEAN:
			DestroyBooleanObject((BooleanObject*)obj);
			return;
		case OBJECT_TYPE_NULL:
			DestroyNullObject((NullObject*)obj);
			return;
		case OBJECT_TYPE_RETURN_VALUE:
			DestroyReturnValueObject((ReturnValueObject*)obj);
			return;
		case OBJECT_TYPE_ERROR:
			DestroyErrorObject((ErrorObject*)obj);
			return;
		case OBJECT_TYPE_FUNCTION:
			DestroyFunctionObject((FunctionObject*)obj);
			return;
	}
	(void)fprintf(stderr, "Unknown object type: %d\n", obj->type);
	assert(false);
}

MONKEY_FILE_LOCAL FunctionObject* rawCreateFunctionObject(FunctionAstOwnership astOwnership,
		IdentifierSpan parameters, BlockStatement* body, Environment* env) {
	FunctionObject* obj = malloc(sizeof(FunctionObject));
	obj->base.type = OBJECT_TYPE_FUNCTION;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	obj->astOwnership = astOwnership;
	obj->parameters = parameters;
	obj->body = body;
	obj->env = env;
	return obj;
}

MONKEY_FILE_LOCAL FunctionObject* copyFunctionObject(FunctionObject* obj) {
	return rawCreateFunctionObject(
			FUNCTION_AST_BORROWED, obj->parameters, obj->body, CopyEnvironment(obj->env));
}

Object* CopyObject(Object* obj) {
	if (obj->freeable == OBJECT_DISALLOW_FREE) {
		return obj;
	}

	switch (obj->type) {
		case OBJECT_TYPE_INTEGER:
			return (Object*)CreateIntegerObject(((IntegerObject*)obj)->value);
		case OBJECT_TYPE_BOOLEAN:
			return (Object*)CreateBooleanObject(((BooleanObject*)obj)->value);
		case OBJECT_TYPE_NULL:
			return (Object*)CreateNullObject();
		case OBJECT_TYPE_RETURN_VALUE:
			return (Object*)CreateReturnValueObject(CopyObject(((ReturnValueObject*)obj)->value));
		case OBJECT_TYPE_ERROR:
			return (Object*)CreateErrorObject(MonkeyStrdup(((ErrorObject*)obj)->message));
		case OBJECT_TYPE_FUNCTION:
			return (Object*)copyFunctionObject((FunctionObject*)obj);
	}
	(void)fprintf(stderr, "Unknown object type: %d\n", obj->type);
	assert(false);
}

IntegerObject* CreateIntegerObject(int64_t value) {
	IntegerObject* obj = malloc(sizeof(IntegerObject));
	obj->base.type = OBJECT_TYPE_INTEGER;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	obj->value = value;
	return obj;
}

char* InspectIntegerObject(const IntegerObject* obj) {
	return MonkeyAsprintf("%" PRId64, obj->value);
}

void DestroyIntegerObject(IntegerObject* obj) {
	free(obj);
}

BooleanObject* CreateBooleanObject(bool value) {
	BooleanObject* obj = malloc(sizeof(BooleanObject));
	obj->base.type = OBJECT_TYPE_BOOLEAN;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	obj->value = value;
	return obj;
}

char* InspectBooleanObject(const BooleanObject* obj) {
	return MonkeyStrdup(obj->value ? "true" : "false");
}

void DestroyBooleanObject(BooleanObject* obj) {
	free(obj);
}

NullObject* CreateNullObject(void) {
	NullObject* obj = malloc(sizeof(NullObject));
	obj->base.type = OBJECT_TYPE_NULL;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	return obj;
}

char* InspectNullObject(const NullObject* obj) {
	(void)obj;
	return MonkeyStrdup("null");
}

void DestroyNullObject(NullObject* obj) {
	free(obj);
}

ReturnValueObject* CreateReturnValueObject(Object* value) {
	ReturnValueObject* obj = malloc(sizeof(ReturnValueObject));
	obj->base.type = OBJECT_TYPE_RETURN_VALUE;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	obj->value = value;
	return obj;
}

char* InspectReturnValueObject(const ReturnValueObject* obj) {
	return InspectObject(obj->value);
}

void DestroyReturnValueObject(ReturnValueObject* obj) {
	DestroyObject(obj->value);
	free(obj);
}

ErrorObject* CreateErrorObject(char* message) {
	ErrorObject* obj = malloc(sizeof(ErrorObject));
	obj->base.type = OBJECT_TYPE_ERROR;
	obj->base.freeable = OBJECT_ALLOW_FREE;
	obj->message = message;
	return obj;
}

char* InspectErrorObject(const ErrorObject* obj) {
	return MonkeyAsprintf("ERROR: %s", obj->message);
}

void DestroyErrorObject(ErrorObject* obj) {
	free(obj->message);
	free(obj);
}

FunctionObject* CreateFunctionObject(FunctionLiteral* func, Environment* env) {
	FunctionObject* obj =
			rawCreateFunctionObject(FUNCTION_AST_OWNED, func->parameters, func->body, env);
	func->parameters = (IdentifierSpan)SPAN_EMPTY;
	func->body = NULL;
	return obj;
}

char* InspectFunctionObject(const FunctionObject* obj) {
	MonkeyStringBuffer out = BUFFER_INIT;
	BUFFER_PUSH(&out, MonkeyStrdup("fn("));
	for (size_t i = 0; i < obj->parameters.length; ++i) {
		if (i > 0) {
			BUFFER_PUSH(&out, MonkeyStrdup(", "));
		}
		BUFFER_PUSH(&out, IdentifierString(obj->parameters.begin[i]));
	}
	BUFFER_PUSH(&out, MonkeyStrdup(") {\n"));
	BUFFER_PUSH(&out, BlockStatementString(obj->body));
	BUFFER_PUSH(&out, MonkeyStrdup("\n}"));
	char* result = MonkeyStringJoin((MonkeyStringSpan)BUFFER_AS_SPAN(out));
	for (size_t i = 0; i < out.length; ++i) {
		free(out.data[i]);
	}
	BUFFER_FREE(out);
	return result;
}

void DestroyFunctionObject(FunctionObject* obj) {
	if (obj->astOwnership == FUNCTION_AST_OWNED) {
		for (size_t i = 0; i < obj->parameters.length; ++i) {
			DestroyIdentifier(obj->parameters.begin[i]);
		}
		free(obj->parameters.begin);
		DestroyBlockStatement(obj->body);
	}
	DestroyEnvironment(obj->env);
	free(obj);
}
