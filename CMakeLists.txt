cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cyd-zephyr-tests)

include(${ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

# Add source files directly to the main app
target_sources(app PRIVATE 
    src/main.c
    components/display_component/display_component.c
    components/wifi_component/wifi_component.c
    components/sntp_component/sntp_component.c
)

# Add include directories
target_include_directories(app PRIVATE
    components/display_component
    components/wifi_component
    components/sntp_component
)