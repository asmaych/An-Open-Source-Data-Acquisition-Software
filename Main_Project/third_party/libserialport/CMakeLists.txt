cmake_minimum_required(VERSION 3.14)
project(libserialport C)

add_library(serialport STATIC
    serialport.c
    timing.c
)

# Platform backend
if (WIN32)
    target_sources(serialport PRIVATE windows.c)
    target_compile_definitions(serialport PUBLIC
        _CRT_SECURE_NO_WARNINGS
        SP_PRIV=
        HAVE_WINDOWS_H=1
        SP_API=
    )
    target_link_libraries(serialport PUBLIC setupapi advapi32)
    target_link_libraries(serialport PUBLIC setupapi advapi32)
elseif (APPLE)
    target_sources(serialport PRIVATE macosx.c)
    find_library(IOKIT_LIB IOKit REQUIRED)
    find_library(COREFOUNDATION_LIB CoreFoundation REQUIRED)
    target_link_libraries(serialport PUBLIC ${IOKIT_LIB} ${COREFOUNDATION_LIB})
elseif (UNIX)
    target_sources(serialport PRIVATE linux.c linux_termios.c)
endif()

# Generate config.h into the build dir
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h [=[
#pragma once
#ifdef _WIN32
  #define SP_PRIV
  #define HAVE_WINDOWS_H 1
#elif defined(__APPLE__)
  #define SP_PRIV __attribute__((visibility("hidden")))
  #define HAVE_IOKIT_SERIAL_IOSS_H 1
#else
  #define SP_PRIV __attribute__((visibility("hidden")))
  #define HAVE_TERMIOS_H 1
  #define HAVE_SYS_IOCTL_H 1
  #define HAVE_SERIAL_H 1
#endif
]=])

# Force-include config.h on non-Windows only
if (NOT WIN32)
    target_compile_options(serialport PRIVATE
        -include ${CMAKE_CURRENT_BINARY_DIR}/config.h
    )
endif()

target_include_directories(serialport PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(serialport PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
