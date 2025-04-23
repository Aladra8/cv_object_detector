cmake_minimum_required(VERSION 3.10)
project(cv_object_detector)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

file(GLOB SOURCES "src/*.cpp")

add_executable(main main.cpp ${SOURCES})
target_link_libraries(main ${OpenCV_LIBS} stdc++fs)
