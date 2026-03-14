# CMake targets for Linux HAL and executable applications.

add_library(lora_linux_hal
    src/linux_spi.cpp
    src/linux_gpio.cpp
    src/linux_delay.cpp
)

target_include_directories(lora_linux_hal
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/common/include
)

target_link_libraries(lora_linux_hal
PUBLIC
    lora_common
)

add_executable(lora_gateway
    src/gateway_main.cpp
)

target_link_libraries(lora_gateway
    lora_linux_hal
)

add_executable(lora_node
    src/node_main.cpp
)

target_link_libraries(lora_node
    lora_linux_hal
)