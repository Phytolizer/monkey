#include <catch.hpp>
#include <memory>
#include <string>

extern "C" {
#include "monkey.h"
}

#include "monkey_wrapper.hpp"

TEST_CASE("Name is Monkey", "[library]") {
	MonkeyPtr lib{CreateMonkey()};

	REQUIRE(std::string("Monkey") == lib->name);
}
