#include <catch2/catch.hpp>
#include <cstdio>
#include <string>

extern "C" {
#include "monkey/stream.h"
}

TEST_CASE("Stream behaves nicely when backed by files", "[stream]") {
	FILE* tempFile = std::tmpfile();
	Stream* stream = StreamFromFile(tempFile);

	const char input[] = "Hello, World!";

	WriteStream(stream, input, sizeof input);

	char buffer[sizeof input];
	RewindStream(stream);
	ReadStream(stream, buffer, sizeof input);

	REQUIRE(std::string{buffer} == std::string{input});

	CloseStream(stream);
}
