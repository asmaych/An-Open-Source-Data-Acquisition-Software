cmake_minimum_required(VERSION 3.10)
project(SerialCommTest)

set(CMAKE_CXX_STANDARD 17)


include_directories(${PROJECT_SOURCE_DIR}/src)

# Add your source files
add_executable(SerialCommTest
    src/main.cpp
    src/SerialComm.cpp
)

# Find and link libserialport
find_library(SERIALPORT_LIB serialport)

if (NOT SERIALPORT_LIB)
    message(FATAL_ERROR "libserialport not found. Please install it.")
endif()

target_link_libraries(SerialCommTest PRIVATE ${SERIALPORT_LIB})


# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif()


# Link libraries
target_link_libraries(SerialCommTest PRIVATE ${wxWidgets_LIBRARIES} ${LIBUSB_LIBRARIES})


