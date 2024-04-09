cmake_minimum_required(VERSION 3.20)

set(qwallet_VERSION 0.1.2)

project(qwallet
        VERSION ${qwallet_VERSION}
        DESCRIPTION "Qubic desktop wallet (Windows/Linux)"
        HOMEPAGE_URL "https://github.com/wilricknl/qwallet"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

## Interpret source as utf-8 on Windows to ensure
## non-ascii characters are read properly.
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

## Static builds on Windows
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# -----------------------------------------------------------------------------
# DEPENDENCIES
# -----------------------------------------------------------------------------

find_package(Catch2 CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

include_directories(include)

# -----------------------------------------------------------------------------
# qwallet
# -----------------------------------------------------------------------------

add_library(
	qwallet_library
	src/wallet.cpp
	src/gui/qwallet.cpp
	src/gui/wallet_window.cpp
	src/gui/window.cpp
	src/network/connection.cpp)
target_link_libraries(
	qwallet_library
	PRIVATE
	cryptopp::cryptopp
	glad::glad
	glfw
	imgui::imgui
	OpenGL::GL
	tl::expected)
if(NOT WIN32)
	target_compile_options(qwallet_library PRIVATE "-march=native")
endif()
# todo (wilricknl): figure out how to include tl header files with library

add_executable(
	qwallet 
	src/main.cpp)
target_link_libraries(
	qwallet
	PRIVATE
	qwallet_library
	glad::glad
	glfw)

# -----------------------------------------------------------------------------
# TEST
# -----------------------------------------------------------------------------

add_executable(
	test_qubic_client
	test/test_qubic_client.cpp)
target_link_libraries(
	test_qubic_client
	PRIVATE
	qwallet_library
	tl::expected)
if(NOT WIN32)
	target_compile_options(test_qubic_client PRIVATE "-march=native")
endif()

add_executable(
	test_client
	test/test_client.cpp)
target_link_libraries(
	test_client
	PRIVATE
	qwallet_library
	tl::expected)

add_executable(
	test_server
	test/test_server.cpp)
target_link_libraries(
	test_server
	PRIVATE
	qwallet_library)

add_executable(
	test_qwallet
	test/test_wallet.cpp)
target_link_libraries(
	test_qwallet
	PRIVATE
	qwallet_library
	Catch2::Catch2
	Catch2::Catch2WithMain)

# -----------------------------------------------------------------------------
# INSTALLATION
# -----------------------------------------------------------------------------

install(TARGETS qwallet)
