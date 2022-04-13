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
	for (const auto TT : TESTS) {
		testLetStatement(program->statements.begin[i], TT.expectedIdentifier);
		i++;
	}
}
