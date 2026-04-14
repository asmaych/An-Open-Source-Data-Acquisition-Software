# Adding a guard to enforce that the project is built out-of-source
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR
			"In-source builds are not allowed.
         Please create a build directory and run cmake from there.")
endif()

cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
project(daq_capstone LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)

set(wxBUILD_SHARED OFF)

FetchContent_Declare(
		wxWidgets
		GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
		GIT_SHALLOW ON
)
FetchContent_MakeAvailable(wxWidgets)

#now add sqlite
add_library(sqlite3 STATIC
		${CMAKE_SOURCE_DIR}/third_party/sqlite3/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
		${CMAKE_SOURCE_DIR}/third_party/sqlite3
)
# ── libserialport (vendored source) ───────────────────────────────────────────
add_subdirectory(third_party/libserialport)

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
		${PROJECT_SOURCE_DIR}/third_party/libserialport
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
		src/ui/CalibrationCompareDialog.cpp
		src/ui/ProjectConfigDialog.cpp
		src/ui/HardwareConfirmDialog.cpp
		src/ui/OpenProjectDialog.cpp
		src/ui/ExportManager.cpp
		src/ui/CalibrationRestorer.cpp
		src/db/DatabaseManager.cpp
		src/sensor/RawReader.cpp
		src/sensor/VoltageReader.cpp
		src/sensor/MappedReader.cpp
)

if (WIN32)
    set_target_properties(daq_capstone PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

#link libserialport and wxwidgets
target_link_libraries(daq_capstone PRIVATE
		wxcore wxaui wxbase
		sqlite3
		serialport)
