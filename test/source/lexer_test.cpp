#include <array>
#include <catch.hpp>

extern "C" {
#include <monkey/lexer.h>
#include <monkey/token.h>
}

#include "monkey_wrapper.hpp"

TEST_CASE("Lexer reports problem for illegal tokens", "[lexer]") {
	constexpr const char INPUT[] = "@";
	const MonkeyPtr monkey{CreateMonkey()};

	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};

	Token token = LexerNextToken(lexer.get());
	const TokenPtr tokenPtr{&token};
	REQUIRE(std::string(TokenTypeText(token.type)) ==
			std::string(TokenTypeText(TOKEN_TYPE_ILLEGAL)));
}

TEST_CASE("Lexer lexes tokens", "[lexer]") {
	constexpr const char INPUT[] = R"mk(
		let five = 6;
		let ten = 10;

		let add = fn(x, y) {
			x + y;
		};

		let result = add(five, ten);
		!-/*5;
		5 < 10 > 5;

		if (5 < 10) {
			return true;
		} else {
			return false;
		}

		10 == 10;
		10 != 9;
	)mk";

	struct Test {
		TokenType expectedType;
		const char* expectedLiteral;
	};

	constexpr Test TESTS[] = {
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_IDENT, "five"},
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_INT, "6"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_IDENT, "ten"},
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_IDENT, "add"},
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_FUNCTION, "fn"},
			{TOKEN_TYPE_LPAREN, "("},
			{TOKEN_TYPE_IDENT, "x"},
			{TOKEN_TYPE_COMMA, ","},
			{TOKEN_TYPE_IDENT, "y"},
			{TOKEN_TYPE_RPAREN, ")"},
			{TOKEN_TYPE_LBRACE, "{"},
			{TOKEN_TYPE_IDENT, "x"},
			{TOKEN_TYPE_PLUS, "+"},
			{TOKEN_TYPE_IDENT, "y"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_RBRACE, "}"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_IDENT, "result"},
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_IDENT, "add"},
			{TOKEN_TYPE_LPAREN, "("},
			{TOKEN_TYPE_IDENT, "five"},
			{TOKEN_TYPE_COMMA, ","},
			{TOKEN_TYPE_IDENT, "ten"},
			{TOKEN_TYPE_RPAREN, ")"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_BANG, "!"},
			{TOKEN_TYPE_MINUS, "-"},
			{TOKEN_TYPE_SLASH, "/"},
			{TOKEN_TYPE_ASTERISK, "*"},
			{TOKEN_TYPE_INT, "5"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_INT, "5"},
			{TOKEN_TYPE_LT, "<"},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_GT, ">"},
			{TOKEN_TYPE_INT, "5"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_IF, "if"},
			{TOKEN_TYPE_LPAREN, "("},
			{TOKEN_TYPE_INT, "5"},
			{TOKEN_TYPE_LT, "<"},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_RPAREN, ")"},
			{TOKEN_TYPE_LBRACE, "{"},
			{TOKEN_TYPE_RETURN, "return"},
			{TOKEN_TYPE_TRUE, "true"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_RBRACE, "}"},
			{TOKEN_TYPE_ELSE, "else"},
			{TOKEN_TYPE_LBRACE, "{"},
			{TOKEN_TYPE_RETURN, "return"},
			{TOKEN_TYPE_FALSE, "false"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_RBRACE, "}"},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_EQ, "=="},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_INT, "10"},
			{TOKEN_TYPE_NOT_EQ, "!="},
			{TOKEN_TYPE_INT, "9"},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_END_OF_FILE, ""},
	};

	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};

	for (const auto tt : TESTS) {
		auto tok = LexerNextToken(lexer.get());
		const TokenPtr tokPtr{&tok};
		REQUIRE(std::string(TokenTypeText(tt.expectedType)) ==
				std::string(TokenTypeText(tok.type)));
		REQUIRE(std::string(tt.expectedLiteral) == tok.literal);
	}
}
