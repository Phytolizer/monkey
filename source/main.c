#include "monkey.h"

#include <stddef.h>
#include <stdio.h>

int main(int argc, const char* argv[]) {
	Monkey lib = CreateMonkey();

	(void)argc;
	(void)argv;

	if (lib.name == NULL) {
		(void)puts("Hello from unknown! (JSON parsing failed in library)");
	} else {
		(void)printf("Hello from %s!", lib.name);
	}
	DestroyMonkey(&lib);
	return 0;
}
