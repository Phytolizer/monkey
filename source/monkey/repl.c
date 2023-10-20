#include "monkey/repl.h"

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/macros.h"
#include "monkey/parser.h"
#include "monkey/stream.h"
#include "monkey/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GETLINE_INITIAL_LENGTH 256

MONKEY_FILE_LOCAL void printParserErrors(Stream* out, MonkeyStringBuffer errors) {
	for (size_t i = 0; i < errors.length; i++) {
		WriteStream(out, "\t", 1);
		WriteStream(out, errors.data[i], strlen(errors.data[i]));
		WriteStream(out, "\n", 1);
	}
}

void MonkeyRepl(MonkeyReplArgs args) {
	char* line = NULL;
	size_t lineCapacity = 0;
	Monkey* monkey = CreateMonkey();
	while (true) {
		WriteStream(args.writer, "> ", 2);
		int64_t lineLength = ReadStreamLine(&line, &lineCapacity, args.reader);
		if (lineLength == -1) {
			WriteStream(args.writer, "\n", 1);
			break;
		}

		Lexer* lexer = CreateLexer(monkey, line);
		Parser* parser = CreateParser(lexer);
		Program* program = ParseProgram(parser);
		MonkeyStringBuffer errors = ParserErrors(parser);
		if (errors.length > 0) {
			printParserErrors(args.writer, errors);
			DestroyProgram(program);
			DestroyParser(parser);
			DestroyLexer(lexer);
			continue;
		}

		char* text = ProgramString(program);
		WriteStream(args.writer, text, strlen(text));
		free(text);
		WriteStream(args.writer, "\n", 1);
		DestroyProgram(program);
		DestroyParser(parser);
		DestroyLexer(lexer);
	}
	free(line);
	DestroyMonkey(monkey);
}
