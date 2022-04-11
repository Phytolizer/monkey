#include "monkey.h"

#include <stddef.h>
#include <stdio.h>

int main(int argc, const char* argv[]) {
	Monkey lib = create_monkey();

	(void)argc;
	(void)argv;

	if (lib.name == NULL) {
		(void)puts("Hello from unknown! (JSON parsing failed in library)");
	} else {
		(void)printf("Hello from %s!", lib.name);
	}
	destroy_monkey(&lib);
	return 0;
}
