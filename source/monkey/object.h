#pragma once

#include <stdbool.h>
#include <stdint.h>

#define OBJECT_TYPES_X \
	X(INTEGER) \
	X(BOOLEAN) \
	X(NULL)

typedef enum {
#define X(x) OBJECT_TYPE_##x,
	OBJECT_TYPES_X
#undef X
} ObjectType;

typedef struct {
	ObjectType type;
} Object;

char* InspectObject(const Object* obj);
void DestroyObject(Object* obj);

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
