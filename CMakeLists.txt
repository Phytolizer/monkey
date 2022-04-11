cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
	monkey
	VERSION 0.1.0
	DESCRIPTION "Short description"
	HOMEPAGE_URL "https://example.com/"
	LANGUAGES C
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
	monkey_lib OBJECT source/monkey.c source/monkey/lexer.c
						source/monkey/string.c source/monkey/token.c
)

target_include_directories(
	monkey_lib ${warning_guard}
	PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(monkey_lib PUBLIC c_std_11)

find_path(HEDLEY_INCLUDE_DIR hedley.h)
if(NOT HEDLEY_INCLUDE_DIR)
	message(FATAL_ERROR "Couldn't find hedley.h (${HEDLEY_INCLUDE_DIR})")
endif()
target_include_directories(monkey_lib SYSTEM PRIVATE "${HEDLEY_INCLUDE_DIR}")

# ---- Declare executable ----

add_executable(monkey_exe source/main.c)
add_executable(monkey::exe ALIAS monkey_exe)

set_target_properties(monkey_exe PROPERTIES OUTPUT_NAME monkey EXPORT_NAME exe)

target_compile_features(monkey_exe PRIVATE c_std_11)

target_link_libraries(monkey_exe PRIVATE monkey_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
	include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT monkey_DEVELOPER_MODE)
	return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
	message(
		AUTHOR_WARNING "Developer mode is intended for developers of monkey"
	)
endif()

include(cmake/dev-mode.cmake)
