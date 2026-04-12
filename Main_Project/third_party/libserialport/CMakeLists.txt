cmake_minimum_required(VERSION 3.14)
project(libserialport C)

add_library(serialport STATIC
        serialport.c
        timing.c
)

# Platform backend
if (WIN32)
    target_sources(serialport PRIVATE windows.c)
    target_compile_definitions(serialport PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(serialport PUBLIC setupapi advapi32)
elseif (APPLE)
    target_sources(serialport PRIVATE macosx.c)
    find_library(IOKIT_LIB IOKit REQUIRED)
    find_library(COREFOUNDATION_LIB CoreFoundation REQUIRED)
    target_link_libraries(serialport PUBLIC ${IOKIT_LIB} ${COREFOUNDATION_LIB})
elseif (UNIX)
    target_sources(serialport PRIVATE linux.c linux_termios.c)
endif()

# config.h — replaces what autoconf would generate
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h [=[
#pragma once

/* Visibility / internal linkage macro that libserialport_internal.h expects */
#define SP_PRIV __attribute__((visibility("hidden")))

#ifdef _WIN32
  #define HAVE_WINDOWS_H 1
#elif defined(__APPLE__)
  #define HAVE_IOKIT_SERIAL_IOSS_H 1
#else
  #define HAVE_TERMIOS_H 1
  #define HAVE_SYS_IOCTL_H 1
  #define HAVE_SERIAL_H 1
#endif
]=])

# Force config.h to be included before any source file
target_compile_options(serialport PRIVATE
        -include ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# Both the build dir (for config.h) and source dir (for internal headers)
# must be on the PRIVATE include path
target_include_directories(serialport PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}   # config.h lives here
        ${CMAKE_CURRENT_SOURCE_DIR}   # libserialport_internal.h lives here
)

# The public header — consumers need this
target_include_directories(serialport PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)