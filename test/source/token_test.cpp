#include <catch2/catch.hpp>

extern "C" {
#include <monkey/token.h>
}

TEST_CASE("TokenTypeText returns NULL for invalid types") {
	REQUIRE(TokenTypeText((TokenType)-1) == NULL);
}
