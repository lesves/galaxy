cmake_minimum_required(VERSION 3.5)

project (galaxy)

set(TARGET_NAME "galaxy")

# === Libraries ===
# OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# TOML++
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# === Executable ===
add_executable(${TARGET_NAME} "main.cpp" "orthtree.hpp")

target_link_libraries( ${TARGET_NAME} ${OpenCV_LIBS} tomlplusplus::tomlplusplus )

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 23)
