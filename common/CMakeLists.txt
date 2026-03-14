# CMake targets for platform-independent common library.

add_library(lora_common
    src/rf95_lora.cpp
    src/lora_packet.cpp
    src/logger.cpp
)

target_include_directories(lora_common
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)