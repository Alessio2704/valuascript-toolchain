cmake_minimum_required(VERSION 3.20)
project(ValuaScript VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable Address and Undefined behavior sanitizers" OFF)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_compile_options("/W4" "/WX" "/MP")

    if (ENABLE_SANITIZERS)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("/fsanitize=address" HAS_ASAN)

        if (HAS_ASAN)
            add_compile_options("/fsanitize=address")
            add_link_options("/fsanitize=address")
        else()
            message(WARNING "ASan requested but not supported by this MSVC compiler!")
        endif()
    endif ()
else ()
    add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror=switch")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(
                "-Wshadow-all"
                "-Wcast-align"
                "-Wunused-lambda-capture"
                "-Wformat=2"
                "-Wnull-dereference"
                "-Wdouble-promotion"
                "-Wimplicit-fallthrough"
                "-Wextra-semi"
                "-Wunreachable-code-aggressive"
        )
    else ()
        add_compile_options(
                "-Wshadow"
                "-Wlogical-op"
                "-Wduplicated-cond"
                "-Wduplicated-branches"
                "-Wformat=2"
        )
    endif ()

    if (ENABLE_SANITIZERS)
        add_compile_options("-fsanitize=address,undefined" "-fno-omit-frame-pointer")
        add_link_options("-fsanitize=address,undefined")

        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_definitions("-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE")
        else ()
            add_definitions("-D_GLIBCXX_ASSERTIONS")
        endif ()
    endif ()
endif ()

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