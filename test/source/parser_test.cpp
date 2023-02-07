#include <catch2/catch.hpp>
#include <memory>

extern "C" {
#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/parser.h"
}

MONKEY_FILE_LOCAL void testLetStatement(Statement* statement, const char* name) {
	char* toklit = StatementTokenLiteral(statement);
	auto toklitPtr = std::unique_ptr<char, decltype(&free)>(toklit, &free);

	REQUIRE(statement->type == STATEMENT_TYPE_LET);
	auto* letStatement = reinterpret_cast<LetStatement*>(statement);
	REQUIRE(std::string(letStatement->identifier->value) == std::string(name));

	char* nameToklit = IdentifierTokenLiteral(letStatement->identifier);
	auto nameToklitPtr = std::unique_ptr<char, decltype(&free)>(nameToklit, &free);
	REQUIRE(std::string(nameToklit) == std::string(name));
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

	Monkey* monkey = CreateMonkey();
	auto monkeyPtr = std::unique_ptr<Monkey, decltype(&DestroyMonkey)>(monkey, &DestroyMonkey);
	Lexer* lexer = CreateLexer(monkey, INPUT);
	auto lexerPtr = std::unique_ptr<Lexer, decltype(&DestroyLexer)>(lexer, &DestroyLexer);
	Parser* parser = CreateParser(lexer);
	auto parserPtr = std::unique_ptr<Parser, decltype(&DestroyParser)>(parser, &DestroyParser);

	Program* program = ParseProgram(parser);
	REQUIRE(program != nullptr);
	auto programPtr = std::unique_ptr<Program, decltype(&DestroyProgram)>(program, &DestroyProgram);

	checkParserErrors(parser);

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

TEST_CASE("Let statement error on missing name", "[parser]") {
	constexpr char INPUT[] = R"mk(
		let = 5;
	)mk";

	Monkey* monkey = CreateMonkey();
	auto monkeyPtr = std::unique_ptr<Monkey, decltype(&DestroyMonkey)>(monkey, &DestroyMonkey);
	Lexer* lexer = CreateLexer(monkey, INPUT);
	auto lexerPtr = std::unique_ptr<Lexer, decltype(&DestroyLexer)>(lexer, &DestroyLexer);
	Parser* parser = CreateParser(lexer);
	auto parserPtr = std::unique_ptr<Parser, decltype(&DestroyParser)>(parser, &DestroyParser);

	Program* program = ParseProgram(parser);
	const auto programPtr = std::unique_ptr<Program, decltype(&DestroyProgram)>(program, &DestroyProgram);

	const MonkeyStringVector errors = ParserErrors(parser);
	CHECK(errors.size > 0);
}

TEST_CASE("Return statements are parsed correctly", "[parser]") {
	constexpr char INPUT[] = R"mk(
		return 5;
		return 10;
		return 993322;
	)mk";

	Monkey* monkey = CreateMonkey();
	auto monkeyPtr = std::unique_ptr<Monkey, decltype(&DestroyMonkey)>(monkey, &DestroyMonkey);
	Lexer* lexer = CreateLexer(monkey, INPUT);
	auto lexerPtr = std::unique_ptr<Lexer, decltype(&DestroyLexer)>(lexer, &DestroyLexer);
	Parser* parser = CreateParser(lexer);
	auto parserPtr = std::unique_ptr<Parser, decltype(&DestroyParser)>(parser, &DestroyParser);

	Program* program = ParseProgram(parser);
	REQUIRE(program != nullptr);
	auto programPtr = std::unique_ptr<Program, decltype(&DestroyProgram)>(program, &DestroyProgram);

	checkParserErrors(parser);

	REQUIRE(program->statements.length == 3);

	for (size_t i = 0; i < program->statements.length; i++) {
		Statement* statement = program->statements.begin[i];
		REQUIRE(statement->type == STATEMENT_TYPE_RETURN);
		char* toklit = StatementTokenLiteral(statement);
		auto toklitPtr = std::unique_ptr<char, decltype(&free)>(toklit, &free);
		REQUIRE(std::string(toklit) == std::string("return"));
	}
}
