#include "monkey/token.h"

#include <stdio.h>
#include <stdlib.h>

const char* TokenTypeText(TokenType type) {
	switch (type) {
#define X(name, string) \
	case TOKEN_TYPE_##name: \
		return string;
		TOKEN_TYPES_X
#undef X
		default:
			fprintf(stderr, "Unknown token type: %d\n", type);
			return NULL;
	}
}

void DestroyToken(Token* token) {
	free(token->literal);
}
