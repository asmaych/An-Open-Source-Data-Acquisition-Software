# Create a static library for libserialport
add_library(libserialport STATIC
    src/serialport.c
    src/serialport_unix.c   # example, add all source files
    src/serialport_win.c    # platform-specific
)

# Include the headers
target_include_directories(libserialport
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specific flags
if(WIN32)
    target_compile_definitions(libserialport PRIVATE _WINDOWS)
elseif(APPLE)
    target_compile_definitions(libserialport PRIVATE _MACOS)
    target_link_libraries(libserialport PRIVATE pthread)
elseif(UNIX)
    target_compile_definitions(libserialport PRIVATE _LINUX)
    target_link_libraries(libserialport PRIVATE pthread)
endif()
