#pragma once

#include "monkey/ast.h"

#include <stdbool.h>
#include <stdint.h>

#define OBJECT_TYPES_X \
	X(INTEGER) \
	X(BOOLEAN) \
	X(NULL) \
	X(RETURN_VALUE) \
	X(ERROR) \
	X(FUNCTION)

typedef enum {
#define X(x) OBJECT_TYPE_##x,
	OBJECT_TYPES_X
#undef X
} ObjectType;

const char* ObjectTypeText(ObjectType type);

typedef enum {
	OBJECT_ALLOW_FREE,
	OBJECT_DISALLOW_FREE,
} ObjectFreeableType;

typedef struct Object {
	ObjectType type;
	/**
	 * @brief freeable is used to determine if the object should be freed when
	 * DestroyObject is called.
	 *
	 * It is always OBJECT_ALLOW_FREE by default.
	 * Will be set to OBJECT_DISALLOW_FREE if the object is not freeable
	 * (the usual case is when the object is a boolean or null).
	 */
	ObjectFreeableType freeable;
} Object;

char* InspectObject(const Object* obj);
void DestroyObject(Object* obj);
Object* CopyObject(Object* obj);

typedef struct {
	Object base;
	int64_t value;
} IntegerObject;

IntegerObject* CreateIntegerObject(int64_t value);
char* InspectIntegerObject(const IntegerObject* obj);
void DestroyIntegerObject(IntegerObject* obj);

typedef struct {
	Object base;
	bool value;
} BooleanObject;

BooleanObject* CreateBooleanObject(bool value);
char* InspectBooleanObject(const BooleanObject* obj);
void DestroyBooleanObject(BooleanObject* obj);

typedef struct {
	Object base;
} NullObject;

NullObject* CreateNullObject(void);
char* InspectNullObject(const NullObject* obj);
void DestroyNullObject(NullObject* obj);

typedef struct {
	Object base;
	Object* value;
} ReturnValueObject;

ReturnValueObject* CreateReturnValueObject(Object* value);
char* InspectReturnValueObject(const ReturnValueObject* obj);
void DestroyReturnValueObject(ReturnValueObject* obj);

typedef struct {
	Object base;
	char* message;
} ErrorObject;

ErrorObject* CreateErrorObject(char* message);
char* InspectErrorObject(const ErrorObject* obj);
void DestroyErrorObject(ErrorObject* obj);

typedef struct {
	Object base;
	IdentifierSpan parameters;
	BlockStatement* body;
	// avoid cyclic include with environment.h here
	struct Environment* env;
} FunctionObject;

FunctionObject* CreateFunctionObject(FunctionLiteral* func, struct Environment* env);
char* InspectFunctionObject(const FunctionObject* obj);
void DestroyFunctionObject(FunctionObject* obj);
