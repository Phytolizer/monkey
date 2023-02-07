#include <catch.hpp>
#include <memory>

extern "C" {
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/parser.h"
}

#include "monkey_wrapper.hpp"

MONKEY_FILE_LOCAL void testLetStatement(Statement* statement, const char* name) {
	const StringPtr toklit{StatementTokenLiteral(statement)};

	REQUIRE(statement->type == STATEMENT_TYPE_LET);
	auto* letStatement = reinterpret_cast<LetStatement*>(statement);
	REQUIRE(std::string(letStatement->identifier->value) == std::string(name));

	const StringPtr nameToklit{IdentifierTokenLiteral(letStatement->identifier)};
	REQUIRE(std::string(nameToklit.get()) == std::string(name));
}

MONKEY_FILE_LOCAL void checkParserErrors(Parser* parser) {
	const MonkeyStringVector errors = ParserErrors(parser);
	for (size_t i = 0; i < errors.size; i++) {
		char* error = errors.data[i];
		FAIL_CHECK(error);
	}
	REQUIRE(errors.size == 0);
}

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
		StringPtr toklit{StatementTokenLiteral(statement)};
		REQUIRE(std::string(toklit.get()) == std::string("return"));
	}
}
