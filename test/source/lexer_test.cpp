#include <array>
#include <catch2/catch.hpp>

extern "C" {
#include <monkey/lexer.h>
#include <monkey/token.h>
}

TEST_CASE("Lexer lexes tokens", "[lexer]") {
	constexpr const char input[] = "=+(){},;";

	struct test {
		TokenType expectedType;
		const char* expectedLiteral;
	};

	constexpr auto tests = std::array<test, 9>{{
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_PLUS, "+"},
			{TOKEN_TYPE_LPAREN, "("},
			{TOKEN_TYPE_RPAREN, ")"},
			{TOKEN_TYPE_LBRACE, "{"},
			{TOKEN_TYPE_RBRACE, "}"},
			{TOKEN_TYPE_COMMA, ","},
			{TOKEN_TYPE_SEMICOLON, ";"},
			{TOKEN_TYPE_END_OF_FILE, ""},
	}};

	auto lexer = CreateLexer(input);
	for (const auto tt : tests) {
		auto tok = LexerNextToken(&lexer);
		auto ptr = std::unique_ptr<Token, void (*)(Token*)>(&tok, &DestroyToken);
		REQUIRE(tt.expectedType == tok.type);
		REQUIRE(std::string(tt.expectedLiteral) == tok.literal);
	}
}
