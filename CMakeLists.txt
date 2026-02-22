cmake_minimum_required(VERSION 3.20)
project(ValuaScript VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_compile_options("/W4" "/WX")
else()
    add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror=switch")
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG "v1.17.0"
)

enable_testing()
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AutoBuild)

add_subdirectory(shared)
add_subdirectory(compiler)
add_subdirectory(virtual_machine)