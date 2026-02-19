function(auto_build_component TARGET_NAME)

    cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

    file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.hpp")
    
    list(FILTER SRC_FILES EXCLUDE REGEX "src/main.cpp$")

    set(HAS_INTERNAL_LIB FALSE)

    if(SRC_FILES)
        add_library(${TARGET_NAME}_lib STATIC ${SRC_FILES} ${HEADER_FILES})
        
        target_include_directories(${TARGET_NAME}_lib PUBLIC 
            src 
            ${CMAKE_SOURCE_DIR}/shared/src
        )

        if(ARG_DEPENDS)
            target_link_libraries(${TARGET_NAME}_lib PUBLIC ${ARG_DEPENDS})
        endif()

        target_compile_features(${TARGET_NAME}_lib PUBLIC cxx_std_20)
        if(MSVC)
            target_compile_options(${TARGET_NAME}_lib PRIVATE "/W4" "/WX")
        else()
            target_compile_options(${TARGET_NAME}_lib PRIVATE "-Wall" "-Wextra" "-Wpedantic")
        endif()

        set(HAS_INTERNAL_LIB TRUE)
        message(STATUS "[${TARGET_NAME}] Core Library configured.")
    else()
        message(STATUS "[${TARGET_NAME}] No library sources found (excluding main.cpp). Skipping library creation.")
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
        add_executable(${TARGET_NAME} "src/main.cpp")
        
        if(HAS_INTERNAL_LIB)
            target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_NAME}_lib)
        elseif(ARG_DEPENDS)
            target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDS})
            target_include_directories(${TARGET_NAME} PRIVATE src ${CMAKE_SOURCE_DIR}/shared/src)
        endif()

        message(STATUS "[${TARGET_NAME}] Executable configured.")
    endif()

    file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "tests/*.cpp")

    if(TEST_FILES)
        if(HAS_INTERNAL_LIB)
            set(TEST_EXE_NAME ${TARGET_NAME}_tests)
            
            add_executable(${TEST_EXE_NAME} ${TEST_FILES})
            
            target_link_libraries(${TEST_EXE_NAME} PRIVATE 
                ${TARGET_NAME}_lib 
                GTest::gtest_main 
                GTest::gmock
            )

            gtest_discover_tests(${TEST_EXE_NAME})
            message(STATUS "[${TARGET_NAME}] Tests configured.")
        else()
             message(STATUS "[${TARGET_NAME}] Tests found but no Core Library to link. Skipping tests.")
        endif()
    endif()

endfunction()