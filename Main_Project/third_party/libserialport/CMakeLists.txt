cmake_minimum_required(VERSION 3.10)
project(libserialport C)

# -----------------------------
# Sources
# -----------------------------
add_library(serialport STATIC
    serialport.c
    timing.c
)

# Platform-specific
if(WIN32)
    target_sources(serialport PRIVATE windows.c)
    target_link_libraries(serialport PRIVATE setupapi)
elseif(APPLE)
    target_sources(serialport PRIVATE macosx.c)
elseif(UNIX)
    target_sources(serialport PRIVATE linux.c linux_termios.c)
endif()

target_include_directories(serialport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
