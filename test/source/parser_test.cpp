#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstddef>
#include <cstdint>
#include <nonstd/variant.hpp>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

extern "C" {
#include <monkey.h>
#include <monkey/ast.h>
#include <monkey/lexer.h>
#include <monkey/parser.h>
#include <monkey/string.h>
}

#include "monkey_wrapper.hpp"

// TODO: test the errors, too

// snuff implicit conversions
struct TestString {
	const char* value;
};
struct TestInt {
	int64_t value;
};
struct TestBool {
	bool value;
};

// generic type for tests
using TestValue = nonstd::variant<TestString, TestInt, TestBool>;
std::ostream& operator<<(std::ostream& os, const TestValue& value) {
	if (const auto* pText = nonstd::get_if<TestString>(&value)) {
		return os << "TestString{" << pText->value << "}";
	}
	if (const auto* pInt = nonstd::get_if<TestInt>(&value)) {
		return os << "TestInt{" << pInt->value << "}";
	}
	if (const auto* pBool = nonstd::get_if<TestBool>(&value)) {
		return os << "TestBool{" << pBool->value << "}";
	}
	return os << "{CORRUPT VALUE}";
}

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

void testBooleanLiteralExpression(Expression* expression, bool value) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_BOOLEAN_LITERAL);
	auto* boolLit = reinterpret_cast<BooleanLiteral*>(expression);

	REQUIRE(boolLit->value == value);
	const StringPtr toklit{BooleanLiteralTokenLiteral(boolLit)};
	REQUIRE(std::string(toklit.get()) == (value ? "true" : "false"));
}

void testLiteralExpression(Expression* expression, TestValue value) {
	REQUIRE(expression != nullptr);
	if (auto* pText = nonstd::get_if<TestString>(&value)) {
		testIdentifierExpression(expression, pText->value);
	} else if (auto* pInt = nonstd::get_if<TestInt>(&value)) {
		testIntegerLiteralExpression(expression, pInt->value);
	} else if (auto* pBool = nonstd::get_if<TestBool>(&value)) {
		testBooleanLiteralExpression(expression, pBool->value);
	} else {
		FAIL("corrupt value");
	}
}

void testPrefixExpression(Expression* expression, const char* op, TestValue value) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_PREFIX);
	auto* prefix = reinterpret_cast<PrefixExpression*>(expression);

	REQUIRE(std::string(op) == prefix->op);
	const StringPtr toklit{PrefixExpressionTokenLiteral(prefix)};
	REQUIRE(std::string(toklit.get()) == op);
	testLiteralExpression(prefix->right, value);
}

void testInfixExpression(Expression* expression, TestValue left, const char* op, TestValue right) {
	REQUIRE(expression != nullptr);
	REQUIRE(expression->type == EXPRESSION_TYPE_INFIX);
	auto* infix = reinterpret_cast<InfixExpression*>(expression);

	REQUIRE(std::string(op) == infix->op);
	const StringPtr toklit{InfixExpressionTokenLiteral(infix)};
	REQUIRE(std::string(toklit.get()) == op);
	testLiteralExpression(infix->left, left);
	testLiteralExpression(infix->right, right);
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
	const MonkeyStringBuffer errors = ParserErrors(parser);
	if (errors.length > 0) {
		CAPTURE(errors);
		FAIL();
	}
}
} // namespace

TEST_CASE("Let statements are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	const char* name;
	TestValue value;
	std::tie(input, name, value) = GENERATE(table<const char*, const char*, TestValue>({
			std::make_tuple("let x = 5;", "x", TestInt{5}),
			std::make_tuple("let y = 10;", "y", TestInt{10}),
			std::make_tuple("let foobar = 838383;", "foobar", TestInt{838383}),
	}));

	CAPTURE(input, name, value);
	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	testLetStatement(program->statements.begin[0], name);
}

TEST_CASE("Let statement errors", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	const char* description;
	std::tie(description, input) = GENERATE(table<const char*, const char*>({
			std::make_tuple("missing variable name", "let = 5;"),
			std::make_tuple("missing = sign", "let x 5;"),
			// FIXME: below is unimplemented
	        // std::make_tuple("missing value", "let x = ;"),
	        // std::make_tuple("missing semicolon", "let x = 5"),
	}));

	CAPTURE(description, input);
	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};

	const MonkeyStringBuffer errors = ParserErrors(parser.get());
	REQUIRE(errors.length > 0);
}

TEST_CASE("Return statements are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	TestValue value;
	std::tie(input, value) = GENERATE(table<const char*, TestValue>({
			std::make_tuple("return 5;", TestInt{5}),
			std::make_tuple("return 10;", TestInt{10}),
			std::make_tuple("return 993322;", TestInt{993322}),
	}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);

	Statement* statement = program->statements.begin[0];
	REQUIRE(statement->type == STATEMENT_TYPE_RETURN);
	const StringPtr toklit{StatementTokenLiteral(statement)};
	REQUIRE(std::string(toklit.get()) == std::string("return"));
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

TEST_CASE("Boolean literal expressions are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	TestBool expected;
	std::tie(input, expected) = GENERATE(table<const char*, TestBool>({
			std::make_tuple("true;", TestBool{true}),
			std::make_tuple("false;", TestBool{false}),
	}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
	testLiteralExpression(stmt->expression, expected);
}

TEST_CASE("Prefix expressions are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	const char* op;
	TestValue value;
	std::tie(input, op, value) = GENERATE(table<const char*, const char*, TestValue>({
			std::make_tuple("!5;", "!", TestInt{5}),
			std::make_tuple("-15;", "-", TestInt{15}),
			std::make_tuple("!true;", "!", TestBool{true}),
			std::make_tuple("!false;", "!", TestBool{false}),
	}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
	testPrefixExpression(stmt->expression, op, value);
}

TEST_CASE("Infix expressions are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};
	const char* input;
	TestValue left;
	const char* op;
	TestValue right;

	std::tie(input, left, op, right) =
			GENERATE(table<const char*, TestValue, const char*, TestValue>({
					std::make_tuple("5 + 5;", TestInt{5}, "+", TestInt{5}),
					std::make_tuple("5 - 5;", TestInt{5}, "-", TestInt{5}),
					std::make_tuple("5 * 5;", TestInt{5}, "*", TestInt{5}),
					std::make_tuple("5 / 5;", TestInt{5}, "/", TestInt{5}),
					std::make_tuple("5 > 5;", TestInt{5}, ">", TestInt{5}),
					std::make_tuple("5 < 5;", TestInt{5}, "<", TestInt{5}),
					std::make_tuple("5 == 5;", TestInt{5}, "==", TestInt{5}),
					std::make_tuple("5 != 5;", TestInt{5}, "!=", TestInt{5}),
					std::make_tuple("true == true;", TestBool{true}, "==", TestBool{true}),
					std::make_tuple("true != false;", TestBool{true}, "!=", TestBool{false}),
					std::make_tuple("false == false;", TestBool{false}, "==", TestBool{false}),
			}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	REQUIRE(program != nullptr);

	checkParserErrors(parser.get());

	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);
	testInfixExpression(stmt->expression, left, op, right);
}

TEST_CASE("Operator precedence is respected", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	const char* expected;

	std::tie(input, expected) = GENERATE(table<const char*, const char*>({
			std::make_tuple("-a * b", "((-a) * b)"),
			std::make_tuple("!-a", "(!(-a))"),
			std::make_tuple("a + b + c", "((a + b) + c)"),
			std::make_tuple("a + b - c", "((a + b) - c)"),
			std::make_tuple("a * b * c", "((a * b) * c)"),
			std::make_tuple("a * b / c", "((a * b) / c)"),
			std::make_tuple("a + b / c", "(a + (b / c))"),
			std::make_tuple("a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"),
			std::make_tuple("3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"),
			std::make_tuple("5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"),
			std::make_tuple("5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"),
			std::make_tuple("3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"),
			std::make_tuple("true", "true"),
			std::make_tuple("false", "false"),
			std::make_tuple("3 > 5 == false", "((3 > 5) == false)"),
			std::make_tuple("3 < 5 == true", "((3 < 5) == true)"),
			std::make_tuple("1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"),
			std::make_tuple("(5 + 5) * 2", "((5 + 5) * 2)"),
			std::make_tuple("2 / (5 + 5)", "(2 / (5 + 5))"),
			std::make_tuple("-(5 + 5)", "(-(5 + 5))"),
			std::make_tuple("!(true == true)", "(!(true == true))"),
			std::make_tuple("a + add(b * c) + d", "((a + add((b * c))) + d)"),
			std::make_tuple("add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
					"add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"),
			std::make_tuple("add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"),
	}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);

	const StringPtr actual{ProgramString(program.get())};
	CHECK(std::string(expected) == actual.get());
}

TEST_CASE("If-expressions are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "if (x < y) { x }";
	const MonkeyPtr monkey{CreateMonkey()};

	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);

	REQUIRE(stmt->expression->type == EXPRESSION_TYPE_IF);
	auto* ifExp = reinterpret_cast<IfExpression*>(stmt->expression);
	testInfixExpression(ifExp->condition, TestString{"x"}, "<", TestString{"y"});

	REQUIRE(ifExp->consequence->statements.length == 1);
	REQUIRE(ifExp->consequence->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* body = reinterpret_cast<ExpressionStatement*>(ifExp->consequence->statements.begin[0]);
	testIdentifierExpression(body->expression, "x");

	REQUIRE(ifExp->alternative == nullptr);
}

TEST_CASE("If/else-expressions are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "if (x < y) { x } else { y }";
	const MonkeyPtr monkey{CreateMonkey()};

	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);

	REQUIRE(stmt->expression->type == EXPRESSION_TYPE_IF);
	auto* ifExp = reinterpret_cast<IfExpression*>(stmt->expression);
	testInfixExpression(ifExp->condition, TestString{"x"}, "<", TestString{"y"});

	REQUIRE(ifExp->consequence->statements.length == 1);
	REQUIRE(ifExp->consequence->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* body = reinterpret_cast<ExpressionStatement*>(ifExp->consequence->statements.begin[0]);
	testIdentifierExpression(body->expression, "x");

	REQUIRE(ifExp->alternative != nullptr);
	REQUIRE(ifExp->alternative->statements.length == 1);
	REQUIRE(ifExp->alternative->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* alternativeBody =
			reinterpret_cast<ExpressionStatement*>(ifExp->alternative->statements.begin[0]);
	testIdentifierExpression(alternativeBody->expression, "y");
}

TEST_CASE("Function literals are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "fn(x, y) { x + y; }";
	const MonkeyPtr monkey{CreateMonkey()};

	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);

	REQUIRE(stmt->expression->type == EXPRESSION_TYPE_FUNCTION_LITERAL);
	auto* lit = reinterpret_cast<FunctionLiteral*>(stmt->expression);
	REQUIRE(lit->parameters.length == 2);
	testLiteralExpression(&lit->parameters.begin[0]->base, TestString{"x"});
	testLiteralExpression(&lit->parameters.begin[1]->base, TestString{"y"});

	REQUIRE(lit->body->statements.length == 1);
	REQUIRE(lit->body->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* bodyStmt = reinterpret_cast<ExpressionStatement*>(lit->body->statements.begin[0]);
	testInfixExpression(bodyStmt->expression, TestString{"x"}, "+", TestString{"y"});
}

TEST_CASE("Function literal parameters are parsed correctly", "[parser]") {
	const MonkeyPtr monkey{CreateMonkey()};

	const char* input;
	using Vec = std::vector<const char*>;
	Vec expectedParams;
	std::tie(input, expectedParams) = GENERATE(table<const char*, Vec>({
			std::make_tuple("fn () {};", Vec{}),
			std::make_tuple("fn (x) {};", Vec{"x"}),
			std::make_tuple("fn (x, y, z) {};", Vec{"x", "y", "z"}),
	}));

	const LexerPtr lexer{CreateLexer(monkey.get(), input)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);

	REQUIRE(stmt->expression->type == EXPRESSION_TYPE_FUNCTION_LITERAL);
	auto* lit = reinterpret_cast<FunctionLiteral*>(stmt->expression);
	REQUIRE(lit->parameters.length == expectedParams.size());
	for (size_t i = 0; i < expectedParams.size(); ++i) {
		testIdentifierExpression(&lit->parameters.begin[i]->base, expectedParams[i]);
	}

	REQUIRE(lit->body->statements.length == 0);
}

TEST_CASE("Call expressions are parsed correctly", "[parser]") {
	constexpr char INPUT[] = "add(1, 2 * 3, 4 + 5)";
	const MonkeyPtr monkey{CreateMonkey()};

	const LexerPtr lexer{CreateLexer(monkey.get(), INPUT)};
	const ParserPtr parser{CreateParser(lexer.get())};

	const ProgramPtr program{ParseProgram(parser.get())};
	checkParserErrors(parser.get());
	REQUIRE(program != nullptr);
	REQUIRE(program->statements.length == 1);
	REQUIRE(program->statements.begin[0]->type == STATEMENT_TYPE_EXPRESSION);
	auto* stmt = reinterpret_cast<ExpressionStatement*>(program->statements.begin[0]);

	REQUIRE(stmt->expression->type == EXPRESSION_TYPE_CALL);
	auto* lit = reinterpret_cast<CallExpression*>(stmt->expression);
	REQUIRE(lit->arguments.length == 3);
	testLiteralExpression(lit->arguments.begin[0], TestInt{1});
	testInfixExpression(lit->arguments.begin[1], TestInt{2}, "*", TestInt{3});
	// NOLINTNEXTLINE(readability-magic-numbers)
	testInfixExpression(lit->arguments.begin[2], TestInt{4}, "+", TestInt{5});
}
