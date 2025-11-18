cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cyd-zephyr-tests)

# Include the display
add_subdirectory(src/display)
add_subdirectory(src/wifi_component)
add_subdirectory(src/sntp_component)

target_sources(app PRIVATE src/main.c)

# Link the say_hello library to the application
target_link_libraries(app PRIVATE display wifi_component sntp_component)
