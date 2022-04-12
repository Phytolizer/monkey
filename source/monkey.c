#include "monkey.h"

#include "monkey/token.h"

#include <hedley.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const char library_name[] = "Monkey";

Monkey CreateMonkey() {
	Monkey lib;
	char* name = malloc(sizeof library_name);
	if (name == NULL) {
		goto exit;
	}

	(void)memcpy(name, library_name, sizeof library_name);

exit:
	lib.token = CreateTokenState();
	lib.name = name;
	return lib;
}

void DestroyMonkey(Monkey* lib) {
	free(HEDLEY_CONST_CAST(void*, lib->name));
	DestroyTokenState(lib->token);
}
