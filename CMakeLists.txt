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

find_package(Qt5 COMPONENTS Widgets Network Concurrent OpenGL REQUIRED)
find_package(OpenCV)

if (NOT OpenCV_FOUND) 
	message(FATAL_ERROR "OpenCV not found")
endif()
include_directories(${OpenCV_INCLUDE_DIRS} )

find_package(PCL 1.7.1 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})

find_package(VTK REQUIRED)
set(VTK_SMP_ENABLE_OPENMP ON)
set(VTK_SMP_ENABLE_STDTHREAD ON)
set(VTK_SMP_ENABLE_TBB ON)
set(VTK_SMP_IMPLEMENTATION_TYPE "OpenMP")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB UI_FILES forms/*.ui)
file(GLOB HEADER_FILES include/*.h)
file(GLOB CXX_FILES src/*.cpp)
file(GLOB RES_FILES res/*.qrc)

add_compile_options(-O3)

add_executable(SonyTOF_client
    ${CXX_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    ${RES_FILES}
)

target_link_libraries(SonyTOF_client Qt5::Widgets Qt5::Network Qt5::Concurrent Qt5::OpenGL ${OpenCV_LIBS} ${PCL_LIBRARIES} ${VTK_LIBRARIES})
vtk_module_autoinit (
TARGETS SonyTOF_client
MODULES ${VTK_MODULES}
)
