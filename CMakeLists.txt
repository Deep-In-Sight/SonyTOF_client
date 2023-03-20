cmake_minimum_required(VERSION 3.1.0)

project(SonyTOF_client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(OpenCV)

if (NOT OpenCV_FOUND) 
	message(FATAL_ERROR "OpenCV not found")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS} )

file(GLOB UI_FILES forms/*.ui)
file(GLOB HEADER_FILES include/*.h)
file(GLOB CXX_FILES src/*.cpp)



add_executable(SonyTOF_client
    ${CXX_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    res/resource.qrc
)

target_link_libraries(SonyTOF_client Qt5::Widgets Qt5::Network ${OpenCV_LIBS})
