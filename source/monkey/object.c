#include "monkey/object.h"

#include "monkey/string.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
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
	return obj;
}

char* InspectNullObject(const NullObject* obj) {
	(void)obj;
	return MonkeyStrdup("null");
}

void DestroyNullObject(NullObject* obj) {
	free(obj);
}
