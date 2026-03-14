# Raspberry Pi ARMHF cross-compilation toolchain.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(LORAWAN_RASPI_ARMHF_GCC arm-linux-gnueabihf-gcc)
find_program(LORAWAN_RASPI_ARMHF_GXX arm-linux-gnueabihf-g++)

if(NOT LORAWAN_RASPI_ARMHF_GCC OR NOT LORAWAN_RASPI_ARMHF_GXX)
    message(FATAL_ERROR "arm-linux-gnueabihf-gcc/g++ not found. Install the Raspberry Pi ARMHF cross-toolchain or override the compiler paths.")
endif()

set(CMAKE_C_COMPILER "${LORAWAN_RASPI_ARMHF_GCC}" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${LORAWAN_RASPI_ARMHF_GXX}" CACHE FILEPATH "C++ compiler" FORCE)

set(RASPI_SYSROOT "" CACHE PATH "Optional Raspberry Pi sysroot used for headers and libraries")

if(RASPI_SYSROOT)
    set(CMAKE_SYSROOT "${RASPI_SYSROOT}")
    list(APPEND CMAKE_FIND_ROOT_PATH "${RASPI_SYSROOT}")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)