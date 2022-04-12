#include "monkey.h"
#include "monkey/repl.h"

#include <stddef.h>
#include <stdio.h>

int main(int argc, const char* argv[]) {
	(void)argc;
	(void)argv;

	MONKEY_REPL(.reader = StreamFromFile(stdin), .writer = StreamFromFile(stdout));
	return 0;
}
