#include <catch.hpp>
#include <string>

extern "C" {
#include "monkey/string.h"
}

#include "monkey_wrapper.hpp"

constexpr int MAGIC = 42;

TEST_CASE("Allocated string printf works", "[string]") {
	const StringPtr result{MonkeyAsprintf("%s %d", "Hello", MAGIC)};
	REQUIRE(std::string{result.get()} == std::string{"Hello 42"});
}
