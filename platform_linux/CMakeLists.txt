# CMake targets for Linux HAL and executable applications.

option(LORAWAN_ALLOW_GPIO_STUB
    "Allow building without libgpiod by using a non-functional GPIO stub backend"
    ON
)

find_path(GPIOD_INCLUDE_DIR gpiod.h)
find_library(GPIOD_LIBRARY NAMES gpiod)

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

if(GPIOD_INCLUDE_DIR AND GPIOD_LIBRARY)
    target_include_directories(lora_linux_hal PRIVATE ${GPIOD_INCLUDE_DIR})
    target_link_libraries(lora_linux_hal PRIVATE ${GPIOD_LIBRARY})
    target_compile_definitions(lora_linux_hal PRIVATE LORAWAN_HAVE_LIBGPIOD=1)
elseif(LORAWAN_ALLOW_GPIO_STUB)
    message(WARNING "libgpiod development files not found. Building with stub GPIO backend; GPIO initialization will fail at runtime.")
else()
    message(FATAL_ERROR "libgpiod development files not found. Install libgpiod-dev or enable LORAWAN_ALLOW_GPIO_STUB.")
endif()

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