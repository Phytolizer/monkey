#pragma once

#include <catch2/catch_tostring.hpp>
#include <cstdlib>
#include <memory>
#include <nonstd/variant.hpp>
#include <sstream>
#include <string>

extern "C" {
#include <monkey/ast.h>
#include <monkey/environment.h>
#include <monkey/lexer.h>
#include <monkey/object.h>
#include <monkey/parser.h>
#include <monkey/stream.h>
#include <monkey/string.h>
}

struct StringDeleter {
	void operator()(char* ptr) {
		std::free(ptr);
	}
};
using StringPtr = std::unique_ptr<char, StringDeleter>;

struct MonkeyDeleter {
	void operator()(Monkey* ptr) {
		DestroyMonkey(ptr);
	}
};
using MonkeyPtr = std::unique_ptr<Monkey, MonkeyDeleter>;

struct LexerDeleter {
	void operator()(Lexer* ptr) {
		DestroyLexer(ptr);
	}
};
using LexerPtr = std::unique_ptr<Lexer, LexerDeleter>;

struct TokenDeleter {
	void operator()(Token* ptr) {
		DestroyToken(ptr);
	}
};
using TokenPtr = std::unique_ptr<Token, TokenDeleter>;

struct ParserDeleter {
	void operator()(Parser* ptr) {
		DestroyParser(ptr);
	}
};
using ParserPtr = std::unique_ptr<Parser, ParserDeleter>;

struct ProgramDeleter {
	void operator()(Program* ptr) {
		DestroyProgram(ptr);
	}
};
using ProgramPtr = std::unique_ptr<Program, ProgramDeleter>;

struct StreamDeleter {
	void operator()(Stream* ptr) {
		CloseStream(ptr);
	}
};
using StreamPtr = std::unique_ptr<Stream, StreamDeleter>;

struct ObjectDeleter {
	void operator()(Object* ptr) {
		DestroyObject(ptr);
	}
};
using ObjectPtr = std::unique_ptr<Object, ObjectDeleter>;

struct EnvironmentDeleter {
	void operator()(Environment* ptr) {
		DestroyEnvironment(ptr);
	}
};
using EnvironmentPtr = std::unique_ptr<Environment, EnvironmentDeleter>;

namespace Catch {
template <> struct StringMaker<MonkeyStringBuffer> {
	// NOLINTNEXTLINE(readability-identifier-naming): catch2 defined this name
	static std::string convert(const MonkeyStringBuffer& value) {
		std::ostringstream result{};
		result << "{\n";
		for (size_t i = 0; i < value.length; ++i) {
			result << "    " << StringMaker<const char*>::convert(value.data[i]) << '\n';
		}
		result << '}';
		return result.str();
	}
};

template <> struct StringMaker<ObjectType> {
	// NOLINTNEXTLINE(readability-identifier-naming): catch2 defined this name
	static std::string convert(ObjectType value) {
		return ObjectTypeText(value);
	}
};
} // namespace Catch

// snuff implicit conversions
struct TestString {
	const char* value;
};
struct TestInt {
	int64_t value;
};
struct TestBool {
	bool value;
};
struct TestNull {};

// generic type for tests
using TestValue = nonstd::variant<TestString, TestInt, TestBool, TestNull>;

namespace Catch {
template <> struct StringMaker<TestValue> {
	// NOLINTNEXTLINE(readability-identifier-naming): catch2 defined this name
	static std::string convert(const TestValue& value) {
		if (const auto* pText = nonstd::get_if<TestString>(&value)) {
			return StringMaker<const char*>::convert(pText->value);
		}
		if (const auto* pInt = nonstd::get_if<TestInt>(&value)) {
			return StringMaker<int64_t>::convert(pInt->value);
		}
		if (const auto* pBool = nonstd::get_if<TestBool>(&value)) {
			return pBool->value ? "true" : "false";
		}
		if (nonstd::get_if<TestNull>(&value) != nullptr) {
			return "null";
		}
		return "{CORRUPT VALUE}";
	}
};
} // namespace Catch
