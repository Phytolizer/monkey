#pragma once

#include <cstdlib>
#include <memory>

extern "C" {
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/parser.h"
#include "monkey/stream.h"
}

struct StringDeleter {
	void operator()(char* ptr) {
		std::free(ptr);
	}
};
using StringPtr = std::unique_ptr<char, StringDeleter>;

struct MonkeyDeleter {
	void operator()(Monkey* ptr) {
		DestroyMonkey(ptr);
	}
};
using MonkeyPtr = std::unique_ptr<Monkey, MonkeyDeleter>;

struct LexerDeleter {
	void operator()(Lexer* ptr) {
		DestroyLexer(ptr);
	}
};
using LexerPtr = std::unique_ptr<Lexer, LexerDeleter>;

struct TokenDeleter {
	void operator()(Token* ptr) {
		DestroyToken(ptr);
	}
};
using TokenPtr = std::unique_ptr<Token, TokenDeleter>;

struct ParserDeleter {
	void operator()(Parser* ptr) {
		DestroyParser(ptr);
	}
};
using ParserPtr = std::unique_ptr<Parser, ParserDeleter>;

struct ProgramDeleter {
	void operator()(Program* ptr) {
		DestroyProgram(ptr);
	}
};
using ProgramPtr = std::unique_ptr<Program, ProgramDeleter>;

struct StreamDeleter {
	void operator()(Stream* ptr) {
		CloseStream(ptr);
	}
};
using StreamPtr = std::unique_ptr<Stream, StreamDeleter>;
