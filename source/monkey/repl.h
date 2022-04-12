#pragma once

#include <stdio.h>

typedef struct {
	FILE* reader;
	FILE* writer;
} MonkeyReplArgs;

/**
 * @brief MonkeyRepl will start a REPL for the Monkey language.
 *
 * @param reader The file to read for input.
 * @param writer The file to write to.
 */
void MonkeyRepl(MonkeyReplArgs args);
#define MONKEY_REPL(...) MonkeyRepl((MonkeyReplArgs){__VA_ARGS__})
