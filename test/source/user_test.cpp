#include <catch.hpp>
#include <memory>

extern "C" {
#include "monkey/user.h"
}

#include "monkey_wrapper.hpp"

TEST_CASE("CurrentUser() returns a valid name", "[user]") {
	StringPtr name{CurrentUser()};
	REQUIRE(name != nullptr);
}
