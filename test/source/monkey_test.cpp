#include <catch2/catch.hpp>
#include <memory>
#include <string>

extern "C" {
#include "monkey.h"
}

TEST_CASE("Name is monkey", "[library]") {
	Monkey lib = create_monkey();
	auto ptr = std::unique_ptr<Monkey, void (*)(Monkey*)>(&lib, &destroy_monkey);

	REQUIRE(std::string("monkey") == lib.name);
}
