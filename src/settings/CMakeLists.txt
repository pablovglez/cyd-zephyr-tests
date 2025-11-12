# Create the library target
add_library(settings STATIC settings.c
)

# Add the library's header file(s)
target_include_directories(settings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(settings PRIVATE zephyr_interface)