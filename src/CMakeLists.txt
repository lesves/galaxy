cmake_minimum_required(VERSION 3.5)

project (galaxy)

set(TARGET_NAME "galaxy")

add_executable(${TARGET_NAME} "main.cpp")
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 23)

# === Libraries ===
include(FetchContent)

# OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( ${TARGET_NAME} ${OpenCV_LIBS} )

if(USE_OPENCV_GRAPHICS)
    add_compile_definitions(USE_OPENCV_GRAPHICS=1)
else()
    # Raylib
    set(FETCHCONTENT_QUIET FALSE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(raylib)

    target_link_libraries( ${TARGET_NAME} raylib )
endif()

# TOML++
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries( ${TARGET_NAME} tomlplusplus::tomlplusplus )
