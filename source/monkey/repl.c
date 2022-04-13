#include "monkey/repl.h"

#include "monkey/lexer.h"

#include <stdbool.h>
#include <stdlib.h>

#define GETLINE_INITIAL_LENGTH 256

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
		while (true) {
			Token token = LexerNextToken(lexer);

			(void)StreamPrintf(args.writer, "{%s, %s}\n", TokenTypeText(token.type), token.literal);
			DestroyToken(&token);
			if (token.type == TOKEN_TYPE_END_OF_FILE) {
				break;
			}
		}
		DestroyLexer(lexer);
	}
	free(line);
	DestroyMonkey(monkey);
}
