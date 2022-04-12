#include <array>
#include <catch2/catch.hpp>

extern "C" {
#include "monkey/repl.h"
}

constexpr std::size_t OUTPUT_BUFFER_SIZE = 1024;

TEST_CASE("REPL prints simple output", "[repl]") {
	char input_text[] = "let five = 6;\n";
	std::array<char, OUTPUT_BUFFER_SIZE> output_text;

	MonkeyReplArgs args = {
			StreamFromText(input_text, sizeof(input_text) - 1),
			StreamFromText(output_text.data(), output_text.size()),
	};
	MonkeyRepl(args);
	args.writer->text[args.writer->text_position] = '\0';

	REQUIRE(std::string(output_text.data()) ==
			"> {LET, let}\n{IDENT, five}\n{=, =}\n{INT, 6}\n{;, ;}\n{EOF, }\n> \n");
}
