#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <string>

extern "C" {
#include <monkey/repl.h>
#include <monkey/stream.h>
}

#include "monkey_wrapper.hpp"

constexpr std::size_t OUTPUT_BUFFER_SIZE = 1024;

TEST_CASE("REPL prints simple output", "[repl]") {
	char inputText[] = "6;\n";
	std::array<char, OUTPUT_BUFFER_SIZE> outputText;

	const MonkeyReplArgs args = {
			StreamFromText(inputText, sizeof(inputText) - 1),
			StreamFromText(outputText.data(), outputText.size()),
	};
	const StreamPtr readerPtr{args.reader};
	const StreamPtr writerPtr{args.writer};

	MonkeyRepl(args);
	args.writer->text[args.writer->textPosition] = '\0';

	REQUIRE(std::string(outputText.data()) == "> 6\n> \n");
}
