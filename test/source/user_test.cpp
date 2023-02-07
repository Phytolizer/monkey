#include <catch.hpp>
#include <memory>

extern "C" {
#include "monkey/user.h"
}

TEST_CASE("CurrentUser() returns a valid name", "[user]") {
	char* name = CurrentUser();
	auto namePtr = std::unique_ptr<char, decltype(&free)>(name, &free);
	REQUIRE(name != nullptr);
}
