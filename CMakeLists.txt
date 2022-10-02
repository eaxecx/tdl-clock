cmake_minimum_required(VERSION 3.5)

project(tdl-clock LANGUAGES C VERSION 1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -O3 -Wno-stringop-overflow)

include(FetchContent)

FetchContent_Declare(
    u8string
    GIT_REPOSITORY "https://github.com/celtrecium/libu8string.git"
    GIT_TAG "master"
)

FetchContent_Declare(
    sbvector
    GIT_REPOSITORY "https://github.com/celtrecium/libsbvector.git"
    GIT_TAG "master"
)

FetchContent_Declare(
    tdl
    GIT_REPOSITORY "https://github.com/celtrecium/tdl.git"
    GIT_TAG "master"
)

FetchContent_MakeAvailable(u8string sbvector)

include_directories(${u8string_SOURCE_DIR}/include)
include_directories(${sbvector_SOURCE_DIR}/include)

FetchContent_MakeAvailable(tdl)

include_directories(${tdl_SOURCE_DIR}/include)

add_executable(tdl-clock main.c)
target_link_libraries(tdl-clock PRIVATE u8string sbvector tdl m)

install(TARGETS tdl-clock DESTINATION bin)