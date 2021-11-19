cmake_minimum_required(VERSION 3.21.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(BUILD_SHARED_LIBS=OFF)
set(Boost_USE_STATIC_LIBS ON)

project(
    video2pixelart-se #name
    VERSION 1.1
    DESCRIPTION "video2pixelart, rewritten in c++. converts videos to pixelart and displays them on the command line"
    LANGUAGES CXX #c++
)

#wut dis
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(
    vtp-se #name
    src/main.c++ #file
)

#headers
target_include_directories(vtp-se PRIVATE src/include/)

target_include_directories(vtp-se PRIVATE lib/)

find_package(Boost REQUIRED COMPONENTS program_options)
target_include_directories(vtp-se PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(vtp-se ${Boost_LIBRARIES})

find_package(OpenCV REQUIRED)
target_include_directories(vtp-se PRIVATE ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(vtp-se ${OpenCV_LIBS} )
