cmake_minimum_required(VERSION 3.20)

set(qwallet_VERSION 0.1.1)

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

find_package(cryptopp CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(include)

# -----------------------------------------------------------------------------
# qwallet
# -----------------------------------------------------------------------------

add_executable(
	qwallet 
	src/main.cpp
	src/wallet.cpp
	src/gui/qwallet.cpp
	src/gui/wallet_window.cpp
	src/gui/window.cpp)
target_link_libraries(
	qwallet
	PRIVATE
	cryptopp::cryptopp
	glad::glad
	glfw
	imgui::imgui
	OpenGL::GL)

# -----------------------------------------------------------------------------
# INSTALLATION
# -----------------------------------------------------------------------------

install(TARGETS qwallet)
