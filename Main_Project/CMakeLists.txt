# Adding a guard to enforce that the project is built out-of-source
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed.
         Please create a build directory and run cmake from there.")
endif()

cmake_minimum_required(VERSION 3.14)
project(daq_capstone LANGUAGES CXX)

find_package(SQLite3 REQUIRED)

include(FetchContent)

set(wxBUILD_SHARED OFF)

FetchContent_Declare(
		wxWidgets
		GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
		GIT_SHALLOW ON
)
FetchContent_MakeAvailable(wxWidgets)

set(CMAKE_CXX_STANDARD 20)

# Detect platform and set library path
if(WIN32)
	set(SERIALPORT_LIB "${CMAKE_SOURCE_DIR}/third_party/libserialport/windows/libserialport.lib")
elseif(APPLE)
	set(SERIALPORT_LIB "${CMAKE_SOURCE_DIR}/third_party/libserialport/mac/libserialport.a")
elseif(UNIX)
	set(SERIALPORT_LIB "${CMAKE_SOURCE_DIR}/third_party/libserialport/linux/libserialport.a")
endif()

# Declare the library as imported STATIC
add_library(serialport STATIC IMPORTED)
set_target_properties(serialport PROPERTIES
		IMPORTED_LOCATION ${SERIALPORT_LIB}
)

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
	${PROJECT_SOURCE_DIR}/include/daq_capstone/db
	${PROJECT_SOURCE_DIR}/include/daq_capstone/processing
	${PROJECT_SOURCE_DIR}/third_party/libserialport/include
)

# Add the source files
add_executable(daq_capstone
	src/App.cpp
	src/sensor/Sensor.cpp
	src/sensor/SensorManager.cpp
	src/sensor/Interpolator.cpp
	src/data/DataTableWindow.cpp
	src/data/LiveDataWindow.cpp
	src/data/GraphWindow.cpp
	src/data/DataSession.cpp
	src/data/Run.cpp
	src/controllers/SessionController.cpp
	src/controllers/DataCollector.cpp
	src/ui/MainFrame.cpp
	src/ui/ProjectPanel.cpp
	src/ui/Toolbar.cpp
	src/serial/SerialComm.cpp
	src/ui/HandshakeDialog.cpp
	src/ui/Sidebar.cpp
	src/ui/Events.cpp
	src/ui/SensorConfigDialog.cpp
	src/ui/AddSensorDialog.cpp
	src/ui/SensorSelectionDialog.cpp
	src/ui/CalibrateSensorDialog.cpp
	src/ui/CalibrationTableDialog.cpp
	src/ui/ProjectConfigDialog.cpp
	src/db/DatabaseManager.cpp
	src/sensor/RawReader.cpp
	src/sensor/VoltageReader.cpp
	src/sensor/MappedReader.cpp
)

target_link_libraries(daq_capstone PRIVATE serialport)
target_link_libraries(daq_capstone PRIVATE wxcore wxaui wxbase)

## Find wxWidgets
#find_package(wxWidgets REQUIRED COMPONENTS core base aui REQUIRED)
#if(wxWidgets_FOUND)
#    include(${wxWidgets_USE_FILE})
#endif()
#
#
## Link wxwidgets
#target_link_libraries(daq_capstone PRIVATE ${wxWidgets_LIBRARIES} ${LIBUSB_LIBRARIES})

# Link sqlite3


target_link_libraries(daq_capstone PRIVATE sqlite3)

