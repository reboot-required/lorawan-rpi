# Raspberry Pi AArch64 cross-compilation toolchain.
# Targets 64-bit Raspberry Pi OS (Pi 5, Pi 4, Pi Zero 2 W with 64-bit OS).

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(LORAWAN_RASPI_AARCH64_GCC aarch64-linux-gnu-gcc)
find_program(LORAWAN_RASPI_AARCH64_GXX aarch64-linux-gnu-g++)

if(NOT LORAWAN_RASPI_AARCH64_GCC OR NOT LORAWAN_RASPI_AARCH64_GXX)
    message(FATAL_ERROR "aarch64-linux-gnu-gcc/g++ not found. Install the AArch64 cross-toolchain:\n  sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu")
endif()

set(CMAKE_C_COMPILER "${LORAWAN_RASPI_AARCH64_GCC}" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${LORAWAN_RASPI_AARCH64_GXX}" CACHE FILEPATH "C++ compiler" FORCE)

set(RASPI_SYSROOT "" CACHE PATH "Optional Raspberry Pi sysroot used for headers and libraries")

if(RASPI_SYSROOT)
    set(CMAKE_SYSROOT "${RASPI_SYSROOT}")
    list(APPEND CMAKE_FIND_ROOT_PATH "${RASPI_SYSROOT}")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
