# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(monkeyTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(
	monkey_test
	source/monkey_test.cpp
	source/lexer_test.cpp
	source/repl_test.cpp
	source/stream_test.cpp
	source/string_test.cpp
	source/user_test.cpp
	source/parser_test.cpp
)
target_link_libraries(monkey_test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(monkey_test PRIVATE monkey_lib)

target_compile_features(monkey_test PRIVATE cxx_std_11)

catch_discover_tests(monkey_test)

# ---- End-of-file commands ----

add_folders(Test)
