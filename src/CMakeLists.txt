cmake_minimum_required(VERSION 3.5)

project (galaxy)

set(TARGET_NAME "galaxy")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(${TARGET_NAME} "main.cpp" "orthtree.hpp")

target_link_libraries( ${TARGET_NAME} ${OpenCV_LIBS} )

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 23)
