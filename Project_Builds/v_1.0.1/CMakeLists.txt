cmake_minimum_required(VERSION 3.10)
project(daq_capstone)

set(CMAKE_CXX_STANDARD 20)


include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/sensor
	${PROJECT_SOURCE_DIR}/src/serial
	${PROJECT_SOURCE_DIR}/src/ui
	${PROJECT_SOURCE_DIR}/src/processing
	${PROJECT_SOURCE_DIR}/include/daq_capstone/
	${PROJECT_SOURCE_DIR}/include/daq_capstone/serial
	${PROJECT_SOURCE_DIR}/include/daq_capstone/sensor
	${PROJECT_SOURCE_DIR}/include/daq_capstone/ui
	${PROJECT_SOURCE_DIR}/include/daq_capstone/processing

)

# Add the source files
add_executable(daq_capstone
    	src/App.cpp
	src/sensor/Sensor.cpp
    	src/ui/MainFrame.cpp
    	src/ui/ProjectPanel.cpp
	src/serial/SerialComm.cpp
	src/ui/HandshakeDialog.cpp
	src/ui/LEDPanel.cpp
	src/ui/Sidebar.cpp
)

# Find and link libserialport
find_library(SERIALPORT_LIB serialport)

if (NOT SERIALPORT_LIB)
    message(FATAL_ERROR "libserialport not found. Please install it.")
endif()

target_link_libraries(daq_capstone PRIVATE ${SERIALPORT_LIB})


# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base aui)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif()


# Link wxwidgets
target_link_libraries(daq_capstone PRIVATE ${wxWidgets_LIBRARIES} ${LIBUSB_LIBRARIES})

