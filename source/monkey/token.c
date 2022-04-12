#include "monkey/token.h"

#include "monkey/macros.h"
#include "monkey/string.h"

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define KEYWORD_COUNT 7

struct MonkeyTokenState {
	char* keywords_text[KEYWORD_COUNT];
	GHashTable* keywords;
};

MONKEY_INTERNAL void fillKeywords(MonkeyTokenState* state) {
	typedef struct {
		TokenType type;
		const char* text;
	} Keyword;
	Keyword keywords[KEYWORD_COUNT] = {
			{TOKEN_TYPE_ELSE, "else"},
			{TOKEN_TYPE_FALSE, "false"},
			{TOKEN_TYPE_FUNCTION, "fn"},
			{TOKEN_TYPE_IF, "if"},
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_RETURN, "return"},
			{TOKEN_TYPE_TRUE, "true"},
	};
	state->keywords = g_hash_table_new(g_str_hash, g_str_equal);
	for (int i = 0; i < KEYWORD_COUNT; i++) {
		state->keywords_text[i] = MonkeyStrdup(keywords[i].text);
		g_hash_table_insert(
				state->keywords, state->keywords_text[i], GINT_TO_POINTER(keywords[i].type));
	}
}

MonkeyTokenState* CreateTokenState(void) {
	MonkeyTokenState* state = malloc(sizeof(MonkeyTokenState));
	fillKeywords(state);
	return state;
}

void DestroyTokenState(MonkeyTokenState* state) {
	g_hash_table_destroy(state->keywords);
	for (size_t i = 0; i < KEYWORD_COUNT; i++) {
		free(state->keywords_text[i]);
	}
	free(state);
}

const char* TokenTypeText(TokenType type) {
	switch (type) {
#define X(name, string) \
	case TOKEN_TYPE_##name: \
		return string;
		TOKEN_TYPES_X
#undef X
	}
	(void)fprintf(stderr, "Unknown token type: %d\n", type);
	assert(false);
	return NULL;
}

void DestroyToken(Token* token) {
	free(token->literal);
}

TokenType LookupIdent(Monkey* monkey, const char* identifier) {
	MonkeyTokenState* state = MonkeyGetTokenState(monkey);
	void* raw = g_hash_table_lookup(state->keywords, identifier);
	if (raw == NULL) {
		return TOKEN_TYPE_IDENT;
	}
	return (TokenType)GPOINTER_TO_INT(raw);
}
