file(RELATIVE_PATH relative_path "/${monkey_INSTALL_CMAKEDIR}"
		"/${CMAKE_INSTALL_BINDIR}/${monkey_NAME}"
)

get_filename_component(prefix "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
set(config_dir "${prefix}/${monkey_INSTALL_CMAKEDIR}")
set(config_file "${config_dir}/monkeyConfig.cmake")

message(STATUS "Installing: ${config_file}")
file(
	WRITE "${config_file}"
	"\
get_filename_component(
	_monkey_executable
	\"\${CMAKE_CURRENT_LIST_DIR}/${relative_path}\"
	ABSOLUTE
)
set(
	MONKEY_EXECUTABLE \"\${_monkey_executable}\"
	CACHE FILEPATH \"Path to the monkey executable\"
)
"
)
list(APPEND CMAKE_INSTALL_MANIFEST_FILES "${config_file}")
