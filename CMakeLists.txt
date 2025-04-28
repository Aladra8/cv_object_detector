cmake_minimum_required(VERSION 3.10)
project(cv_object_detector)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(cv_object_detector 
    main.cpp 
    src/detection.cpp 
    src/evaluate.cpp
)
add_executable(synthetic_test src/synthetic_test.cpp)
target_link_libraries(synthetic_test ${OpenCV_LIBS})

target_link_libraries(cv_object_detector ${OpenCV_LIBS} stdc++fs)
