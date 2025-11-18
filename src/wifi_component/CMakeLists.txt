# Create the library target
add_library(wifi_component PUBLIC wifi_component.c
)

# Add the library's header file(s)
target_include_directories(wifi_component PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(wifi_component PRIVATE zephyr_interface)
