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
void testIntegerObject(const Object* object, int64_t expected) {
	REQUIRE(object != nullptr);
	REQUIRE(object->type == OBJECT_TYPE_INTEGER);
	const auto* integer = reinterpret_cast<const IntegerObject*>(object);
	REQUIRE(integer->value == expected);
}

void testBooleanObject(const Object* object, bool expected) {
	REQUIRE(object != nullptr);
	REQUIRE(object->type == OBJECT_TYPE_BOOLEAN);
	const auto* boolean = reinterpret_cast<const BooleanObject*>(object);
	REQUIRE(boolean->value == expected);
}

ObjectPtr testEval(Monkey* monkey, const char* input) {
	const LexerPtr lexer{CreateLexer(monkey, input)};
	const ParserPtr parser{CreateParser(lexer.get())};
	const ProgramPtr program{ParseProgram(parser.get())};

	return ObjectPtr{Eval(monkey, &program->base)};
}
} // namespace

TEST_CASE("Integer expressions", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	int64_t expected;
	std::tie(input, expected) = GENERATE(table<const char*, int64_t>({
			std::make_tuple("5", 5),
			std::make_tuple("10", 10),
	}));

	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testIntegerObject(evaluated.get(), expected);
}

TEST_CASE("Boolean literals", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	bool expected;
	std::tie(input, expected) = GENERATE(table<const char*, bool>({
			std::make_tuple("true", true),
			std::make_tuple("false", false),
	}));

	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testBooleanObject(evaluated.get(), expected);
}
