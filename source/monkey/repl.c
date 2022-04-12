#include "monkey/repl.h"

#include "monkey/lexer.h"

#include <stdbool.h>
#include <stdlib.h>

#define GETLINE_INITIAL_LENGTH 256

MONKEY_FILE_LOCAL long GetLine(char** linep, size_t* lenp, FILE* reader) {
	if (feof(reader)) {
		return -1;
	}

	if (*linep == NULL) {
		*lenp = GETLINE_INITIAL_LENGTH;
		*linep = malloc(*lenp);
	}

	char* line = *linep;
	size_t len = *lenp;
	size_t pos = 0;

	while (true) {
		int c = fgetc(reader);
		if (c == EOF) {
			if (pos == 0) {
				return -1;
			}
			break;
		}

		if (c == '\n') {
			break;
		}

		if (pos == len) {
			len *= 2;
			line = realloc(line, len);
		}

		line[pos++] = (char)c;
	}

	line[pos] = '\0';
	return (long)pos;
}

void MonkeyRepl(MonkeyReplArgs args) {
	char* line = NULL;
	size_t lineCapacity = 0;
	Monkey* monkey = CreateMonkey();
	while (true) {
		printf("> ");
		long lineLength = GetLine(&line, &lineCapacity, args.reader);
		if (lineLength == -1) {
			(void)fputc('\n', args.writer);
			break;
		}

		Lexer lexer = CreateLexer(monkey, line);
		while (true) {
			Token token = LexerNextToken(&lexer);

			(void)fprintf(args.writer, "{%s, %s}\n", TokenTypeText(token.type), token.literal);
			DestroyToken(&token);
			if (token.type == TOKEN_TYPE_END_OF_FILE) {
				break;
			}
		}
		DestroyLexer(&lexer);
	}
	free(line);
	DestroyMonkey(monkey);
}
