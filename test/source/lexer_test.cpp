#include <array>
#include <catch2/catch.hpp>

extern "C" {
#include <monkey/lexer.h>
#include <monkey/token.h>
}

TEST_CASE("Lexer lexes tokens", "[lexer]") {
	constexpr const char input[] = R"mk(
		let five = 5;
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
	)mk";

	struct test {
		TokenType expectedType;
		const char* expectedLiteral;
	};

	constexpr test tests[] = {
			{TOKEN_TYPE_LET, "let"},
			{TOKEN_TYPE_IDENT, "five"},
			{TOKEN_TYPE_ASSIGN, "="},
			{TOKEN_TYPE_INT, "5"},
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
			{TOKEN_TYPE_END_OF_FILE, ""},
	};

	Monkey monkey = CreateMonkey();
	auto monkey_ptr = std::unique_ptr<Monkey, void (*)(Monkey*)>(&monkey, &DestroyMonkey);

	auto lexer = CreateLexer(&monkey, input);
	auto lexer_ptr = std::unique_ptr<Lexer, void (*)(Lexer*)>(&lexer, &DestroyLexer);

	for (const auto tt : tests) {
		auto tok = LexerNextToken(&lexer);
		auto tok_ptr = std::unique_ptr<Token, void (*)(Token*)>(&tok, &DestroyToken);
		REQUIRE(std::string(TokenTypeText(tt.expectedType)) ==
				std::string(TokenTypeText(tok.type)));
		REQUIRE(std::string(tt.expectedLiteral) == tok.literal);
	}
}
