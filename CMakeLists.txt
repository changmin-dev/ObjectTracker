cmake_minimum_required(VERSION 3.7)
project(object_tracker)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED)

set(SOURCE_FILES ObjectTracker.cpp ObjectTracker.h test.cpp)

add_executable(object_tracker ${SOURCE_FILES})

target_link_libraries( object_tracker ${OpenCV_LIBS} )