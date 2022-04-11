include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package monkey)

install(TARGETS monkey_exe RUNTIME COMPONENT monkey_Runtime)

write_basic_package_version_file(
	"${package}ConfigVersion.cmake" COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(monkey_INSTALL_CMAKEDIR
	"${CMAKE_INSTALL_DATADIR}/${package}"
	CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(monkey_INSTALL_CMAKEDIR)

install(
	FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
	DESTINATION "${monkey_INSTALL_CMAKEDIR}"
	COMPONENT monkey_Development
)

# Export variables for the install script to use
install(
	CODE "
set(monkey_NAME [[$<TARGET_FILE_NAME:monkey_exe>]])
set(monkey_INSTALL_CMAKEDIR [[${monkey_INSTALL_CMAKEDIR}]])
set(CMAKE_INSTALL_BINDIR [[${CMAKE_INSTALL_BINDIR}]])
"
	COMPONENT monkey_Development
)

install(SCRIPT cmake/install-script.cmake COMPONENT monkey_Development)

if(PROJECT_IS_TOP_LEVEL)
	include(CPack)
endif()
