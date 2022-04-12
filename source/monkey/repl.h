#pragma once

#include <stdio.h>

/**
 * @brief MonkeyReplArgs is a struct that holds the arguments for the REPL.
 */
typedef struct {
	FILE* reader;
	FILE* writer;
} MonkeyReplArgs;

/**
 * @brief MonkeyRepl will start a REPL for the Monkey language.
 *
 * @param args The reader and writer.
 */
void MonkeyRepl(MonkeyReplArgs args);
#define MONKEY_REPL(...) MonkeyRepl((MonkeyReplArgs){__VA_ARGS__})
