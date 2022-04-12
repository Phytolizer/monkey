#include <catch2/catch.hpp>
#include <memory>
#include <string>

extern "C" {
#include "monkey.h"
}

TEST_CASE("Name is Monkey", "[library]") {
	Monkey lib = CreateMonkey();
	auto ptr = std::unique_ptr<Monkey, void (*)(Monkey*)>(&lib, &DestroyMonkey);

	REQUIRE(std::string("Monkey") == lib.name);
}
