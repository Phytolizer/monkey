#include <catch2/catch.hpp>
#include <cstdio>
#include <string>

extern "C" {
#include "monkey/stream.h"
#include "monkey/string.h"
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

	RewindStream(stream);
	char* dynamicBuffer = nullptr;
	size_t dynamicBufferSize = 0;
	ReadStreamLine(&dynamicBuffer, &dynamicBufferSize, stream);
	std::unique_ptr<char, decltype(&free)> dynamicBufferPtr(dynamicBuffer, &free);

	REQUIRE(std::string{dynamicBuffer} == std::string{INPUT});

	RewindStream(stream);
	StreamPrintf(stream, "Hello... %s!", "World");

	RewindStream(stream);
	auto expected = std::string{"Hello... World!"};
	auto buffer2 = std::unique_ptr<char[]>(new char[expected.size() + 1]);
	ReadStream(stream, buffer2.get(), expected.size());
	buffer2[expected.size()] = '\0';

	REQUIRE(std::string{buffer2.get()} == expected);
}

TEST_CASE("Stream behaves nicely when backed by a string", "[stream]") {
	const char INPUT[] = "Hello, World!";
	char* dynamicInput = MonkeyStrdup(INPUT);
	std::unique_ptr<char, decltype(&free)> dynamicInputPtr(dynamicInput, &free);
	Stream* stream = StreamFromText(dynamicInput, sizeof INPUT);
	std::unique_ptr<Stream, decltype(&CloseStream)> streamPtr(stream, &CloseStream);

	char buffer[sizeof INPUT];
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});

	RewindStream(stream);
	ReadStream(stream, buffer, sizeof INPUT);

	REQUIRE(std::string{buffer} == std::string{INPUT});
}
