cmake_minimum_required(VERSION 3.25)

# ---------------------------------------------------------------------------
# External dependency locations
# ---------------------------------------------------------------------------

# OpenSiv3D 0.6.16 installation directory.
set(SIV3D_ROOT "C:/Software/OpenSiv3D_0.6.16" CACHE PATH "OpenSiv3D install directory")

# Source trees used to build the third-party libraries with the static CRT.
set(THIRD_PARTY_SOURCE_ROOT "F:/codes/sources" CACHE PATH "Directory containing googletest, spdlog and yaml-cpp sources")

project(Siv3D_Template VERSION 0.1.0 LANGUAGES CXX RC)

# Export compile commands to compile_commands.json (Ninja / Makefile generators only).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MSVC)
	message(FATAL_ERROR "OpenSiv3D 0.6.16 for Windows requires the MSVC toolchain.")
endif()

if(NOT EXISTS "${SIV3D_ROOT}/include/Siv3D.hpp")
	message(FATAL_ERROR "Siv3D was not found at SIV3D_ROOT='${SIV3D_ROOT}'. "
		"Set it with -DSIV3D_ROOT=<path-to-OpenSiv3D_0.6.16>.")
endif()

foreach(dependency googletest spdlog yaml-cpp)
	if(NOT EXISTS "${THIRD_PARTY_SOURCE_ROOT}/${dependency}/CMakeLists.txt")
		message(FATAL_ERROR "The source of '${dependency}' was not found at "
			"THIRD_PARTY_SOURCE_ROOT='${THIRD_PARTY_SOURCE_ROOT}'. "
			"Set it with -DTHIRD_PARTY_SOURCE_ROOT=<path>.")
	endif()
endforeach()

# Siv3D's static libraries are built against the static CRT (/MT, /MTd),
# so every target must use the same runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Third-party libraries
# ---------------------------------------------------------------------------

# GoogleTest (built from source with the static CRT).
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_build_tests OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
add_subdirectory(
	"${THIRD_PARTY_SOURCE_ROOT}/googletest"
	"${CMAKE_BINARY_DIR}/third_party/googletest"
	EXCLUDE_FROM_ALL
)

# spdlog (static library using its bundled fmt).
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory(
	"${THIRD_PARTY_SOURCE_ROOT}/spdlog"
	"${CMAKE_BINARY_DIR}/third_party/spdlog"
	EXCLUDE_FROM_ALL
)

# yaml-cpp (static library).
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)
set(YAML_CPP_USE_STRICT_FLAGS OFF CACHE BOOL "" FORCE)
set(YAML_MSVC_SHARED_RT OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(
	"${THIRD_PARTY_SOURCE_ROOT}/yaml-cpp"
	"${CMAKE_BINARY_DIR}/third_party/yaml-cpp"
	EXCLUDE_FROM_ALL
)

# ---------------------------------------------------------------------------
# Application core library
# ---------------------------------------------------------------------------

add_library(siv3d_template_core STATIC
	src/config.cc
	src/logging.cc
	src/version.cc
)

target_include_directories(siv3d_template_core PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(siv3d_template_core PUBLIC cxx_std_23)

target_compile_definitions(siv3d_template_core PRIVATE
	SIV3D_TEMPLATE_VERSION="${PROJECT_VERSION}"
)

target_compile_options(siv3d_template_core PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:/W4>
	$<$<COMPILE_LANGUAGE:CXX>:/permissive->
	$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
	$<$<COMPILE_LANGUAGE:CXX>:/utf-8>
	$<$<COMPILE_LANGUAGE:CXX>:/MP>
)

target_link_libraries(siv3d_template_core PUBLIC
	spdlog::spdlog
	yaml-cpp::yaml-cpp
)

# ---------------------------------------------------------------------------
# Siv3D application
# ---------------------------------------------------------------------------

add_executable(Siv3D_Template WIN32
	src/Main.cpp
	App/Resource.rc
)

target_include_directories(Siv3D_Template PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${SIV3D_ROOT}/include"
	"${SIV3D_ROOT}/include/ThirdParty"
)

target_link_directories(Siv3D_Template PRIVATE
	"${SIV3D_ROOT}/lib/Windows"
)

# Siv3D's dependency libraries are linked automatically through the
# #pragma comment(lib, ...) directives in Siv3D/Windows/Libraries.hpp.
target_link_libraries(Siv3D_Template PRIVATE
	siv3d_template_core
	"$<IF:$<CONFIG:Debug>,${SIV3D_ROOT}/lib/Windows/Siv3D_d.lib,${SIV3D_ROOT}/lib/Windows/Siv3D.lib>"
	delayimp
)

target_compile_definitions(Siv3D_Template PRIVATE
	_WINDOWS
	_ENABLE_EXTENDED_ALIGNED_STORAGE
	_SILENCE_CXX20_CISO646_REMOVED_WARNING
	_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
	_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
)

target_compile_options(Siv3D_Template PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:/W4>
	$<$<COMPILE_LANGUAGE:CXX>:/permissive->
	$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
	$<$<COMPILE_LANGUAGE:CXX>:/utf-8>
	$<$<COMPILE_LANGUAGE:CXX>:/MP>
	$<$<COMPILE_LANGUAGE:CXX>:/wd26451>
	$<$<COMPILE_LANGUAGE:CXX>:/wd26812>
)

target_link_options(Siv3D_Template PRIVATE
	/DELAYLOAD:advapi32.dll
	/DELAYLOAD:crypt32.dll
	/DELAYLOAD:dwmapi.dll
	/DELAYLOAD:gdi32.dll
	/DELAYLOAD:imm32.dll
	/DELAYLOAD:ole32.dll
	/DELAYLOAD:oleaut32.dll
	/DELAYLOAD:opengl32.dll
	/DELAYLOAD:shell32.dll
	/DELAYLOAD:shlwapi.dll
	/DELAYLOAD:user32.dll
	/DELAYLOAD:winmm.dll
	/DELAYLOAD:ws2_32.dll
	$<$<CONFIG:Release>:/DEBUG>
)

set_property(TARGET Siv3D_Template PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Use pch.h as a precompiled header, as in the Siv3D template.
target_precompile_headers(Siv3D_Template PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/include/siv3d_template/pch.h"
)

set_target_properties(Siv3D_Template PROPERTIES
	OUTPUT_NAME_DEBUG "Siv3D_Template(debug)"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate/${PROJECT_NAME}/$<CONFIG>"
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/App"
)

# Copy the executable into the App folder (assets are located under App).
# A separate ALL target restores the copy even when the executable itself
# does not need to be relinked (for example after cleaning App/).
add_custom_target(copy_app_executable ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:Siv3D_Template>"
		"${CMAKE_CURRENT_SOURCE_DIR}/App"
	DEPENDS Siv3D_Template
	COMMENT "Copying ${PROJECT_NAME} executable to App/"
	VERBATIM
)

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------

option(SIV3D_TEMPLATE_BUILD_TESTS "Build the unit tests" ON)

if(SIV3D_TEMPLATE_BUILD_TESTS)
	enable_testing()

	add_executable(siv3d_template_tests
		tests/main.cpp
		tests/config_test.cc
		tests/logging_test.cc
		tests/version_test.cc
	)

	target_link_libraries(siv3d_template_tests PRIVATE
		siv3d_template_core
		gtest
	)

	target_compile_options(siv3d_template_tests PRIVATE
		$<$<COMPILE_LANGUAGE:CXX>:/W4>
		$<$<COMPILE_LANGUAGE:CXX>:/permissive->
		$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
		$<$<COMPILE_LANGUAGE:CXX>:/utf-8>
		$<$<COMPILE_LANGUAGE:CXX>:/MP>
	)

	set_target_properties(siv3d_template_tests PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate/${PROJECT_NAME}/Tests/$<CONFIG>"
	)

	add_test(NAME siv3d_template_tests COMMAND siv3d_template_tests)
endif()

# ---------------------------------------------------------------------------
# Tooling
# ---------------------------------------------------------------------------

# Also place compile_commands.json in the project root.
# (The Visual Studio generator does not support CMAKE_EXPORT_COMPILE_COMMANDS.)
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
	add_custom_target(export_compile_commands ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_BINARY_DIR}/compile_commands.json"
			"${CMAKE_SOURCE_DIR}/compile_commands.json"
		COMMENT "Copying compile_commands.json to the project root"
		VERBATIM
	)
endif()
