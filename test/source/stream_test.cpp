#include <catch.hpp>
#include <cstdio>
#include <string>

extern "C" {
#include "monkey/stream.h"
#include "monkey/string.h"
}

#include "monkey_wrapper.hpp"

TEST_CASE("Stream behaves nicely when backed by files", "[stream]") {
	FILE* tempFile;
#ifdef _WIN32
	REQUIRE(tmpfile_s(&tempFile) == 0);
#else
	tempFile = std::tmpfile();
#endif
	const StreamPtr streamPtr{StreamFromFile(tempFile)};
	Stream* stream = streamPtr.get();

	constexpr char INPUT[] = "Hello, World!";

	WriteStream(stream, INPUT, sizeof INPUT);

	char buffer[sizeof INPUT];
	RewindStream(stream);
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});

	RewindStream(stream);
	char* dynamicBuffer = nullptr;
	size_t dynamicBufferSize = 0;
	ReadStreamLine(&dynamicBuffer, &dynamicBufferSize, stream);
	const StringPtr dynamicBufferPtr{dynamicBuffer};

	REQUIRE(std::string{dynamicBuffer} == std::string{INPUT});

	RewindStream(stream);
	StreamPrintf(stream, "Hello... %s!", "World");

	RewindStream(stream);
	auto expected = std::string{"Hello... World!"};
	auto buffer2 = std::make_unique<char[]>(expected.size() + 1);
	ReadStream(stream, buffer2.get(), expected.size());
	buffer2[expected.size()] = '\0';

	REQUIRE(std::string{buffer2.get()} == expected);
}

TEST_CASE("Stream behaves nicely when backed by a string", "[stream]") {
	constexpr char INPUT[] = "Hello, World!";
	const StringPtr dynamicInput{MonkeyStrdup(INPUT)};
	const StreamPtr streamPtr{StreamFromText(dynamicInput.get(), sizeof INPUT)};
	Stream* stream = streamPtr.get();

	char buffer[sizeof INPUT];
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});

	RewindStream(stream);
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});
}
