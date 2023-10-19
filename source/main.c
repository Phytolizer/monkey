#include "monkey/repl.h"
#include "monkey/stream.h"
#include "monkey/user.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char* argv[]) {
	(void)argc;
	(void)argv;

	char* user = CurrentUser();
	if (user == NULL) {
		(void)fprintf(stderr, "Could not get current user.\n");
		return EXIT_FAILURE;
	}
	printf("Hello %s! This is the Monkey programming language!\n", user);
	free(user);
	printf("Feel free to type in commands\n");
	MONKEY_REPL(.reader = StreamFromFile(stdin), .writer = StreamFromFile(stdout));
	return EXIT_SUCCESS;
}
