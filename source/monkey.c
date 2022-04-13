#include "monkey.h"

#include "monkey/macros.h"
#include "monkey/token.h"

#include <hedley.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MONKEY_INTERNAL const char LIBRARY_NAME[] = "Monkey";

typedef struct {
	Monkey base;
	MonkeyTokenState* token;
} MonkeyImpl;

Monkey* CreateMonkey() {
	MonkeyImpl* impl = malloc(sizeof(MonkeyImpl));
	char* name = malloc(sizeof LIBRARY_NAME);
	(void)memcpy(name, LIBRARY_NAME, sizeof LIBRARY_NAME);
	impl->base.name = name;
	impl->token = CreateTokenState();
	return (Monkey*)impl;
}

MonkeyTokenState* MonkeyGetTokenState(Monkey* monkey) {
	MonkeyImpl* impl = (MonkeyImpl*)monkey;
	return impl->token;
}

void DestroyMonkey(Monkey* lib) {
	MonkeyImpl* impl = (MonkeyImpl*)lib;
	free(HEDLEY_CONST_CAST(void*, lib->name));
	DestroyTokenState(impl->token);
	free(impl);
}
