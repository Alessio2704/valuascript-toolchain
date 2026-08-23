cmake_minimum_required(VERSION 3.23)
project(ValuaScript VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable Address and Undefined behavior sanitizers" OFF)
message(STATUS "CHECK: ENABLE_SANITIZERS is currently: ${ENABLE_SANITIZERS}")

add_library(valua_compiler_flags INTERFACE)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    target_compile_options(valua_compiler_flags INTERFACE "/W4" "/WX" "/MP" "/bigobj")

    if (ENABLE_SANITIZERS)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("/fsanitize=address" HAS_ASAN)

        if (HAS_ASAN)
            target_compile_options(valua_compiler_flags INTERFACE "/fsanitize=address")
            target_link_options(valua_compiler_flags INTERFACE "/fsanitize=address")
        else ()
            message(WARNING "ASan requested but not supported by this MSVC compiler!")
        endif ()
    endif ()
else ()
    target_compile_options(valua_compiler_flags INTERFACE "-Wall" "-Wextra" "-Wpedantic" "-Werror")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(valua_compiler_flags INTERFACE
                "-Wshadow-all"
                "-Wcast-align"
                "-Wunused-lambda-capture"
                "-Wformat=2"
                "-Wnull-dereference"
                "-Wdouble-promotion"
                "-Wimplicit-fallthrough"
                "-Wextra-semi"
                "-Wunreachable-code-aggressive"
                "-Wconversion"
                "-Wsign-conversion"
                "-Wnon-virtual-dtor"
                "-Woverloaded-virtual"
        )
    else ()
        target_compile_options(valua_compiler_flags INTERFACE
                "-Wshadow"
                "-Wlogical-op"
                "-Wduplicated-cond"
                "-Wduplicated-branches"
                "-Wformat=2"
                "-Wconversion"
                "-Wsign-conversion"
                "-Wnon-virtual-dtor"
                "-Woverloaded-virtual"
                "-Wno-missing-field-initializers"
                "-Wno-maybe-uninitialized"
        )
    endif ()

    if (ENABLE_SANITIZERS)
        target_compile_options(valua_compiler_flags INTERFACE
                "-fsanitize=address,undefined"
                "-fno-omit-frame-pointer"
                "-fno-sanitize-recover=all"
        )
        target_link_options(valua_compiler_flags INTERFACE
                "-fsanitize=address,undefined"
                "-fno-sanitize-recover=all"
        )

        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_definitions(valua_compiler_flags INTERFACE _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE)
        else ()
            target_compile_definitions(valua_compiler_flags INTERFACE _GLIBCXX_ASSERTIONS)
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