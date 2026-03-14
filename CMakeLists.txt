# Root CMake configuration for lorawan-rpi.

cmake_minimum_required(VERSION 3.31)

project(rf95_lora LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

add_subdirectory(common)
add_subdirectory(platform_linux)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()