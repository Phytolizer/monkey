#include "monkey/parser.h"

#include <stdlib.h>

struct Parser {
	Lexer* lexer;

	Token currentToken;
	Token peekToken;
};

MONKEY_FILE_LOCAL void nextToken(Parser* parser) {
	DestroyToken(&parser->currentToken);
	parser->currentToken = parser->peekToken;
	parser->peekToken = LexerNextToken(parser->lexer);
}

Parser* CreateParser(Lexer* lexer) {
	Parser* parser = calloc(1, sizeof(Parser));
	parser->lexer = lexer;

	nextToken(parser);
	nextToken(parser);

	return parser;
}

Program* ParseProgram(Parser* parser) {
	(void)parser;
	return NULL;
}

void DestroyParser(Parser* parser) {
	DestroyToken(&parser->currentToken);
	DestroyToken(&parser->peekToken);
	free(parser);
}
