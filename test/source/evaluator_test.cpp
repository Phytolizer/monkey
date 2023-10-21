#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstdint>
#include <tuple>

extern "C" {
#include <monkey.h>
#include <monkey/evaluator.h>
#include <monkey/lexer.h>
#include <monkey/object.h>
#include <monkey/parser.h>
}

#include "monkey_wrapper.hpp"

namespace {
void testIntegerObject(Object* object, int64_t expected) {
	REQUIRE(object != nullptr);
	REQUIRE(object->type == OBJECT_TYPE_INTEGER);
	auto* integer = reinterpret_cast<IntegerObject*>(object);
	REQUIRE(integer->value == expected);
}

void testBooleanObject(Object* object, bool expected) {
	REQUIRE(object != nullptr);
	REQUIRE(object->type == OBJECT_TYPE_BOOLEAN);
	auto* boolean = reinterpret_cast<BooleanObject*>(object);
	REQUIRE(boolean->value == expected);
}

ObjectPtr testEval(const char* input) {
	const MonkeyPtr monkey{CreateMonkey()};
	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};
	const ProgramPtr program{ParseProgram(parser.get())};

	return ObjectPtr{Eval(&program->base)};
}
} // namespace

TEST_CASE("Integer expressions", "[evaluator]") {
	const char* input;
	int64_t expected;
	std::tie(input, expected) = GENERATE(table<const char*, int64_t>({
			std::make_tuple("5", 5),
			std::make_tuple("10", 10),
	}));

	const ObjectPtr evaluated = testEval(input);
	testIntegerObject(evaluated.get(), expected);
}

TEST_CASE("Boolean literals", "[evaluator]") {
	const char* input;
	bool expected;
	std::tie(input, expected) = GENERATE(table<const char*, bool>({
			std::make_tuple("true", true),
			std::make_tuple("false", false),
	}));

	const ObjectPtr evaluated = testEval(input);
	testBooleanObject(evaluated.get(), expected);
}
