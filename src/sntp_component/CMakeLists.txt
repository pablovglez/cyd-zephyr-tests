# Create the library target
add_library(sntp_component STATIC sntp_component.c
)

# Add the library's header file(s)
target_include_directories(sntp_component PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(sntp_component PRIVATE zephyr_interface)

