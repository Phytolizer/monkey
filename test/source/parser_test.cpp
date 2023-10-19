#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

extern "C" {
#include <monkey.h>
#include <monkey/ast.h>
#include <monkey/lexer.h>
#include <monkey/parser.h>
#include <monkey/string.h>
}

#include "monkey_wrapper.hpp"

namespace {
void testIdentifierExpression(Expression* expression, const char* name) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_IDENTIFIER);
	auto* ident = reinterpret_cast<Identifier*>(expression);

	REQUIRE(std::string(ident->value) == std::string(name));
	const StringPtr toklit{IdentifierTokenLiteral(ident)};
	REQUIRE(std::string(toklit.get()) == std::string(name));
}

void testIntegerLiteralExpression(Expression* expression, int64_t value) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_INTEGER_LITERAL);
	auto* intlit = reinterpret_cast<IntegerLiteral*>(expression);

	REQUIRE(intlit->value == value);
	const StringPtr toklit{IntegerLiteralTokenLiteral(intlit)};
	REQUIRE(std::string(toklit.get()) == std::to_string(value));
}

void testPrefixExpression(Expression* expression, const char* op, int64_t value) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_PREFIX);
	auto* prefix = reinterpret_cast<PrefixExpression*>(expression);

	REQUIRE(std::string(op) == prefix->op);
	const StringPtr toklit{PrefixExpressionTokenLiteral(prefix)};
	REQUIRE(std::string(toklit.get()) == op);
	testIntegerLiteralExpression(prefix->right, value);
}

void testLetStatement(Statement* statement, const char* name) {
	const StringPtr toklit{StatementTokenLiteral(statement)};

	REQUIRE(statement->type == STATEMENT_TYPE_LET);
	auto* letStatement = reinterpret_cast<LetStatement*>(statement);
	REQUIRE(std::string(letStatement->identifier->value) == std::string(name));

	const StringPtr nameToklit{IdentifierTokenLiteral(letStatement->identifier)};
	REQUIRE(std::string(nameToklit.get()) == std::string(name));
}

void checkParserErrors(Parser* parser) {
	const MonkeyStringVector rawErrors = ParserErrors(parser);
	if (rawErrors.size > 0) {
		auto errors = std::vector<char*>(rawErrors.data, rawErrors.data + rawErrors.size);
		CAPTURE(errors);
		FAIL();
	}
}
} // namespace

TEST_CASE("Let statements are parsed correctly", "[parser]") {
	constexpr char INPUT[] = R"mk(
		let x = 5;
		let y = 10;
		let foobar = 838383;
	)mk";

	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 3);

	struct Test {
		const char* expectedIdentifier;
	};

	constexpr Test TESTS[] = {
			{"x"},
			{"y"},
			{"foobar"},
	};

	std::size_t i = 0;
	for (const auto tt : TESTS) {
		testLetStatement(program->statements.begin[i], tt.expectedIdentifier);
		i++;
	}
}

TEST_CASE("Let statement errors", "[parser]") {
	const auto doTest = [](const char* input) {
		const MonkeyPtr monkey{CreateMonkey()};
		const LexerPtr lexer{CreateLexer(monkey.get(), input)};
		const ParserPtr parser{CreateParser(lexer.get())};

		const ProgramPtr program{ParseProgram(parser.get())};

		const MonkeyStringVector errors = ParserErrors(parser.get());
		CHECK(errors.size > 0);
	};

	SECTION("missing variable name") {
		doTest("let = 5;");
	}
	SECTION("missing = sign") {
		doTest("let x 5;");
	}
	// FIXME: below is unimplemented
	// SECTION("missing value") {
	// 	doTest("let x = ;");
	// }
	// SECTION("missing semicolon") {
	// 	doTest("let x = 5");
	// }
}

TEST_CASE("Return statements are parsed correctly", "[parser]") {
	constexpr char INPUT[] = R"mk(
		return 5;
		return 10;
		return 993322;
	)mk";

	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 3);

	for (size_t i = 0; i < program->statements.length; i++) {
		Statement* statement = program->statements.begin[i];
		REQUIRE(statement->type == STATEMENT_TYPE_RETURN);
		const StringPtr toklit{StatementTokenLiteral(statement)};
		REQUIRE(std::string(toklit.get()) == std::string("return"));
	}
}

TEST_CASE("Identifier expressions are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "foobar;";

	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
	testIdentifierExpression(stmt->expression, "foobar");
}

TEST_CASE("Integer literal expressions are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "5;";

	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
	testIntegerLiteralExpression(stmt->expression, 5); // NOLINT(readability-magic-numbers)
}

TEST_CASE("Prefix expressions are parsed correctly", "[parser]") {
	typedef struct {
		const char* input;
		const char* op;
		int64_t value;
	} PrefixTest;

	constexpr PrefixTest TESTS[] = {
			{"!5;", "!", 5},
			{"-15;", "-", 15},
	};

	const MonkeyPtr monkey{CreateMonkey()};

	for (auto tt : TESTS) {
		const LexerPtr lexer{CreateLexer(monkey.get(), tt.input)};
		const ParserPtr parser{CreateParser(lexer.get())};

		const ProgramPtr program{ParseProgram(parser.get())};
		REQUIRE(program != nullptr);

		checkParserErrors(parser.get());

		REQUIRE(program->statements.length == 1);
		REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
		auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
		testPrefixExpression(stmt->expression, tt.op, tt.value);
	}
}
