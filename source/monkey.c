#include "monkey.h"

#include "monkey/macros.h"
#include "monkey/object.h"
#include "monkey/token.h"

#include <hedley.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

MONKEY_INTERNAL const char LIBRARY_NAME[] = "Monkey";

typedef struct {
	Monkey base;
	MonkeyTokenState* token;
	MonkeyInternedObjects interns;
} MonkeyImpl;

Monkey* CreateMonkey(void) {
	MonkeyImpl* impl = malloc(sizeof(MonkeyImpl));
	char* name = malloc(sizeof LIBRARY_NAME);
	(void)memcpy(name, LIBRARY_NAME, sizeof LIBRARY_NAME);
	impl->base.name = name;
	impl->token = CreateTokenState();
	impl->interns.trueObj = (Object*)CreateBooleanObject(true);
	impl->interns.trueObj->freeable = OBJECT_DISALLOW_FREE;
	impl->interns.falseObj = (Object*)CreateBooleanObject(false);
	impl->interns.falseObj->freeable = OBJECT_DISALLOW_FREE;
	impl->interns.nullObj = (Object*)CreateNullObject();
	impl->interns.nullObj->freeable = OBJECT_DISALLOW_FREE;
	return (Monkey*)impl;
}

MonkeyTokenState* MonkeyGetTokenState(Monkey* monkey) {
	MonkeyImpl* impl = (MonkeyImpl*)monkey;
	return impl->token;
}

MonkeyInternedObjects MonkeyGetInterns(Monkey* monkey) {
	MonkeyImpl* impl = (MonkeyImpl*)monkey;
	return impl->interns;
}

void DestroyMonkey(Monkey* lib) {
	MonkeyImpl* impl = (MonkeyImpl*)lib;
	impl->interns.trueObj->freeable = OBJECT_ALLOW_FREE;
	DestroyObject(impl->interns.trueObj);
	impl->interns.falseObj->freeable = OBJECT_ALLOW_FREE;
	DestroyObject(impl->interns.falseObj);
	impl->interns.nullObj->freeable = OBJECT_ALLOW_FREE;
	DestroyObject(impl->interns.nullObj);
	free(HEDLEY_CONST_CAST(void*, lib->name));
	DestroyTokenState(impl->token);
	free(impl);
}
