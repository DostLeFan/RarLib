cmake_minimum_required(VERSION 3.16)

project(
	RarLib
	VERSION 1.0.0
	DESCRIPTION "Cross-platform C++ wrapper library for RAR/WinRAR compression"
	LANGUAGES CXX
)

# =============================================================================
# Options
# =============================================================================

option(RARLIB_BUILD_SHARED    "Build RarLib as a shared library"          OFF)
option(RARLIB_BUILD_TESTS     "Build RarLib tests"                        OFF)
option(RARLIB_HEADER_ONLY     "Expose a header-only INTERFACE target"     ON)
option(RARLIB_INSTALL         "Generate install rules"                    ON)

# =============================================================================
# C++ standard
# =============================================================================

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# =============================================================================
# Sources
# =============================================================================

set(RARLIB_HEADERS
	include/RarLib/Rar.hpp
	include/RarLib/OS.hpp
)

set(RARLIB_SOURCES
	src/RarLib/Rar.cpp
)

# =============================================================================
# Compiled library target (static or shared)
# =============================================================================

if(RARLIB_BUILD_SHARED)
	add_library(RarLib SHARED ${RARLIB_SOURCES} ${RARLIB_HEADERS})
else()
	add_library(RarLib STATIC ${RARLIB_SOURCES} ${RARLIB_HEADERS})
endif()

add_library(RarLib::RarLib ALIAS RarLib)

target_include_directories(RarLib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(RarLib PUBLIC cxx_std_17)

# Platform-specific link dependencies
if(WIN32)
	target_link_libraries(RarLib PRIVATE Advapi32)  # For RegOpenKeyExA
endif()

# Compiler warnings
if(MSVC)
	target_compile_options(RarLib PRIVATE /W4)
else()
	target_compile_options(RarLib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# Header-only INTERFACE target
# =============================================================================

# Note: the header-only target requires the user to compile Rar.cpp themselves,
# or to define RARLIB_HEADER_ONLY and include the implementation manually.
# It is provided for convenience (e.g. single-file integration).

if(RARLIB_HEADER_ONLY)
	add_library(RarLib_HeaderOnly INTERFACE)
	add_library(RarLib::HeaderOnly ALIAS RarLib_HeaderOnly)
	
	target_include_directories(RarLib_HeaderOnly
		INTERFACE
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:include>
	)
	
	target_compile_features(RarLib_HeaderOnly INTERFACE cxx_std_17)
endif()

# =============================================================================
# Tests
# =============================================================================

if(RARLIB_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

# =============================================================================
# Install rules
# =============================================================================

if(RARLIB_INSTALL)
	include(GNUInstallDirs)
	include(CMakePackageConfigHelpers)
	
	# Install compiled library and headers
	install(
		TARGETS RarLib
		EXPORT  RarLibTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
	
	install(
		DIRECTORY include/RarLib
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
	
	# Install header-only target
	if(RARLIB_HEADER_ONLY)
		install(
			TARGETS RarLib_HeaderOnly
			EXPORT  RarLibTargets
		)
	endif()
	
	# Export targets file
	install(
		EXPORT      RarLibTargets
		FILE        RarLibTargets.cmake
		NAMESPACE   RarLib::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RarLib
	)
	
	# Generate RarLibConfigVersion.cmake
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/RarLibConfigVersion.cmake"
		VERSION       ${PROJECT_VERSION}
		COMPATIBILITY SameMajorVersion
	)
	
	# Generate RarLibConfig.cmake from template
	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/RarLibConfig.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/RarLibConfig.cmake"
		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RarLib
	)
	
	# Install both config files
	install(
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/RarLibConfig.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/RarLibConfigVersion.cmake"
		DESTINATION
			${CMAKE_INSTALL_LIBDIR}/cmake/RarLib
	)
endif()