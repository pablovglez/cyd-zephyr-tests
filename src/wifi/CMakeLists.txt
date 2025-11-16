# Create the library target
add_library(wifi STATIC wifi.c
)

# Add the library's header file(s)
target_include_directories(wifi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(wifi PRIVATE zephyr_interface)
