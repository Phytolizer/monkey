#include "monkey.h"

#include <hedley.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const char library_name[] = "Monkey";

Monkey create_monkey() {
	Monkey lib;
	char* name = malloc(sizeof library_name);
	if (name == NULL) {
		goto exit;
	}

	(void)memcpy(name, library_name, sizeof library_name);

exit:
	lib.name = name;
	return lib;
}

void destroy_monkey(Monkey* lib) {
	free(HEDLEY_CONST_CAST(void*, lib->name));
}
