cmake_minimum_required(VERSION 3.10)
project(daq_capstone)

set(CMAKE_CXX_STANDARD 20)


include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/sensor
	${PROJECT_SOURCE_DIR}/src/serial
	${PROJECT_SOURCE_DIR}/src/data
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
	src/sensor/SensorManager.cpp
	src/data/DataTableWindow.cpp
	src/data/LiveDataWindow.cpp
	src/data/GraphWindow.cpp
	src/data/DataSession.cpp
	src/ui/MainFrame.cpp
	src/ui/ProjectPanel.cpp
	src/serial/SerialComm.cpp
	src/ui/HandshakeDialog.cpp
	src/ui/Sidebar.cpp
	src/ui/Events.cpp
	src/ui/SensorConfigDialog.cpp
	src/ui/AddSensorDialog.cpp

)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SERIALPORT REQUIRED libserialport)

target_include_directories(daq_capstone PRIVATE ${SERIALPORT_INCLUDE_DIRS})
link_directories(${SERIALPORT_LIBRARY_DIRS})
add_definitions(${SERIALPORT_CFLAGS_OTHER})

target_link_libraries(daq_capstone PRIVATE ${SERIALPORT_LIBRARIES})


# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base aui REQUIRED)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif()


# Link wxwidgets
target_link_libraries(daq_capstone PRIVATE ${wxWidgets_LIBRARIES} ${LIBUSB_LIBRARIES})

