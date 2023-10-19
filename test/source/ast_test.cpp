
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/string.h"
#include "monkey/token.h"
#include "span.h"
}

#include "monkey_wrapper.hpp"

TEST_CASE("AST can be pretty-printed", "[ast]") {
	const MonkeyPtr monkey{CreateMonkey()};
	Statement* rawStatements[] = {
			&CreateLetStatement(Token{TOKEN_TYPE_LET, MonkeyStrdup("let")},
					CreateIdentifier(
							Token{TOKEN_TYPE_IDENT, MonkeyStrdup("myVar")}, MonkeyStrdup("myVar")),
					&CreateIdentifier(Token{TOKEN_TYPE_IDENT, MonkeyStrdup("anotherVar")},
							MonkeyStrdup("anotherVar"))
							 ->base)
					 ->base,
	};
	Statement** statements = static_cast<Statement**>(malloc(sizeof(Statement*)));
	memcpy(statements, rawStatements, sizeof(rawStatements));
	const ProgramPtr program{CreateProgram(SPAN_WITH_LENGTH(statements, 1))};

	const StringPtr actual{ProgramString(program.get())};
	REQUIRE(actual.get() == std::string{"let myVar = anotherVar;"});
}
