cmake_minimum_required(VERSION 3.21.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(
    video2pixelart-se #name
    VERSION 1.0
    DESCRIPTION "video2pixelart, rewritten in c++. converts videos to pixelart and displays them on the command line"
    LANGUAGES CXX #c++
)

#wut dis
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(
    vtp-se #name
    src/main.cpp #file
)

find_package(Boost REQUIRED COMPONENTS program_options)
target_include_directories(vtp-se PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(vtp-se ${Boost_LIBRARIES})

find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(vtp-se ${OpenCV_LIBS} )
