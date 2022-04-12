#include "monkey.h"

#include "monkey/macros.h"
#include "monkey/token.h"

#include <hedley.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MONKEY_INTERNAL const char library_name[] = "Monkey";

typedef struct {
	Monkey base;
	MonkeyTokenState* token;
} MonkeyImpl;

Monkey* CreateMonkey() {
	MonkeyImpl* impl = malloc(sizeof(MonkeyImpl));
	char* name = malloc(sizeof library_name);
	(void)memcpy(name, library_name, sizeof library_name);
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
