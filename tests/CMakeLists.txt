# Tests for platform-independent components.

add_executable(lora_packet_tests
    lora_packet_tests.cpp
)

target_link_libraries(lora_packet_tests
    PRIVATE
        lora_common
)

set(LORAWAN_CAN_RUN_TESTS ON)
get_filename_component(_lora_cxx_compiler_name "${CMAKE_CXX_COMPILER}" NAME)

if(CMAKE_CROSSCOMPILING)
    set(LORAWAN_CAN_RUN_TESTS OFF)
elseif(_lora_cxx_compiler_name MATCHES "arm-linux-gnueabihf")
    set(LORAWAN_CAN_RUN_TESTS OFF)
endif()

if(LORAWAN_CAN_RUN_TESTS)
    add_test(
        NAME lora_packet_tests
        COMMAND lora_packet_tests
    )
else()
    message(STATUS "Built lora_packet_tests but skipped CTest registration for target toolchain")
    add_test(
        NAME lora_packet_tests_skipped
        COMMAND ${CMAKE_COMMAND} -E echo "Skipping lora_packet_tests runtime execution for target toolchain"
    )
endif()
