#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstdint>
#include <nonstd/variant.hpp>
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

void testNullObject(const Object* object) {
	REQUIRE(object != nullptr);
	REQUIRE(object->type == OBJECT_TYPE_NULL);
}

void testObject(const Object* object, TestValue expected) {
	if (auto* pInt = nonstd::get_if<TestInt>(&expected)) {
		testIntegerObject(object, pInt->value);
	} else if (auto* pBool = nonstd::get_if<TestBool>(&expected)) {
		testBooleanObject(object, pBool->value);
	} else if (nonstd::get_if<TestNull>(&expected) != nullptr) {
		testNullObject(object);
	} else {
		FAIL("corrupt value");
	}
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
			std::make_tuple("-5", -5),
			std::make_tuple("-10", -10),
			std::make_tuple("5 + 5 + 5 + 5 - 10", 10),
			std::make_tuple("2 * 2 * 2 * 2 * 2", 32),
			std::make_tuple("-50 + 100 + -50", 0),
			std::make_tuple("5 * 2 + 10", 20),
			std::make_tuple("5 + 2 * 10", 25),
			std::make_tuple("20 + 2 * -10", 0),
			std::make_tuple("50 / 2 * 2 + 10", 60),
			std::make_tuple("2 * (5 + 10)", 30),
			std::make_tuple("3 * 3 * 3 + 10", 37),
			std::make_tuple("3 * (3 * 3) + 10", 37),
			std::make_tuple("(5 + 10 * 2 + 15 / 3) * 2 + -10", 50),
	}));

	CAPTURE(input, expected);
	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testIntegerObject(evaluated.get(), expected);
}

TEST_CASE("Boolean expressions", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	bool expected;
	std::tie(input, expected) = GENERATE(table<const char*, bool>({
			std::make_tuple("true", true),
			std::make_tuple("false", false),
			std::make_tuple("1 < 2", true),
			std::make_tuple("1 > 2", false),
			std::make_tuple("1 < 1", false),
			std::make_tuple("1 > 1", false),
			std::make_tuple("1 == 1", true),
			std::make_tuple("1 != 1", false),
			std::make_tuple("1 == 2", false),
			std::make_tuple("1 != 2", true),
			std::make_tuple("true == true", true),
			std::make_tuple("false == false", true),
			std::make_tuple("true == false", false),
			std::make_tuple("true != false", true),
			std::make_tuple("false != true", true),
			std::make_tuple("(1 < 2) == true", true),
			std::make_tuple("(1 < 2) == false", false),
			std::make_tuple("(1 > 2) == true", false),
			std::make_tuple("(1 > 2) == false", true),
	}));

	CAPTURE(input, expected);
	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testBooleanObject(evaluated.get(), expected);
}

TEST_CASE("Prefix expressions", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	TestValue expected;
	std::tie(input, expected) = GENERATE(table<const char*, TestValue>({
			std::make_tuple("!true", TestBool{false}),
			std::make_tuple("!false", TestBool{true}),
			std::make_tuple("!5", TestBool{false}),
			std::make_tuple("!!true", TestBool{true}),
			std::make_tuple("!!false", TestBool{false}),
			std::make_tuple("!!5", TestBool{true}),
	}));

	CAPTURE(input, expected);
	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testObject(evaluated.get(), expected);
}

TEST_CASE("If/else expressions", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	TestValue expected;
	std::tie(input, expected) = GENERATE(table<const char*, TestValue>({
			std::make_tuple("if (true) { 10 }", TestInt{10}),
			std::make_tuple("if (false) { 10 }", TestNull{}),
			std::make_tuple("if (1) { 10 }", TestInt{10}),
			std::make_tuple("if (1 < 2) { 10 }", TestInt{10}),
			std::make_tuple("if (1 > 2) { 10 }", TestNull{}),
			std::make_tuple("if (1 > 2) { 10 } else { 20 }", TestInt{20}),
			std::make_tuple("if (1 < 2) { 10 } else { 20 }", TestInt{10}),
	}));

	CAPTURE(input, expected);
	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testObject(evaluated.get(), expected);
}

TEST_CASE("Return statements", "[evaluator]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	TestValue expected;
	std::tie(input, expected) = GENERATE(table<const char*, TestValue>({
			std::make_tuple("return 10;", TestInt{10}),
			std::make_tuple("return 10; 9;", TestInt{10}),
			std::make_tuple("return 2 * 5; 9;", TestInt{10}),
			std::make_tuple("9; return 2 * 5; 9;", TestInt{10}),
			std::make_tuple(R"mk(
				if (10 > 1) {
					if (10 > 1) {
						return 10;
					}

					return 1;
				}
			)mk",
					TestInt{10}),
	}));

	CAPTURE(input, expected);
	const ObjectPtr evaluated = testEval(monkey.get(), input);
	testObject(evaluated.get(), expected);
}
