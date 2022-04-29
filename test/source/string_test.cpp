#include <catch2/catch.hpp>
#include <string>

extern "C" {
#include "monkey/string.h"
}

constexpr int MAGIC = 42;

TEST_CASE("Allocated string printf works", "[string]") {
	char* result = MonkeyAsprintf("%s %d", "Hello", MAGIC);
	auto resultPtr = std::unique_ptr<char, decltype(&free)>(result, free);
	REQUIRE(result == std::string{"Hello 42"});
}
