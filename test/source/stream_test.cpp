#include <catch2/catch.hpp>
#include <cstdio>
#include <string>

extern "C" {
#include "monkey/stream.h"
}

TEST_CASE("Stream behaves nicely when backed by files", "[stream]") {
	FILE* tempFile;
#ifdef _WIN32
	REQUIRE(tmpfile_s(&tempFile) == 0);
#else
	tempFile = std::tmpfile();
#endif
	Stream* stream = StreamFromFile(tempFile);
	std::unique_ptr<Stream, decltype(&CloseStream)> streamPtr(stream, &CloseStream);

	const char INPUT[] = "Hello, World!";

	WriteStream(stream, INPUT, sizeof INPUT);

	char buffer[sizeof INPUT];
	RewindStream(stream);
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});
}
