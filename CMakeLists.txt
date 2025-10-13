cmake_minimum_required(VERSION 3.20.2 FATAL_ERROR)

project(java-openzl CXX C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JNI REQUIRED)

set(OPENZL_BUILD_ALL OFF)
set(OPENZL_BUILD_SHARED_LIBS ON)
set(OPENZL_BUILD_TESTS OFF)
set(OPENZL_BUILD_BENCHMARKS OFF)
set(OPENZL_BUILD_PARQUET_TOOLS OFF)
set(OPENZL_BUILD_PYTHON_EXT OFF)
set(OPENZL_BUILD_CPP ON)
set(OPENZL_BUILD_CUSTOM_PARSERS ON)
set(OPENZL_BUILD_TOOLS OFF)
set(OPENZL_BUILD_CLI OFF)
set(OPENZL_BUILD_EXAMPLES OFF)
set(OPENZL_INSTALL OFF)

add_subdirectory(../openzl ${CMAKE_CURRENT_BINARY_DIR}/openzl)

add_library(openzl_jni SHARED
    src/main/native/openzl_jni.c
)

set_target_properties(openzl_jni PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "openzl_jni"
)

target_include_directories(openzl_jni PRIVATE
    ${JNI_INCLUDE_DIRS}
    ../openzl/include/openzl
    ../openzl/src
    ../openzl/cpp/include
)

target_link_libraries(openzl_jni PRIVATE
    openzl
    openzl_cpp
    custom_parsers
    csv_parser
    sddl_profile
    parquet_graph
)

if(WIN32)
    set_target_properties(openzl_jni PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
    # Windows-specific compiler flags
    target_compile_definitions(openzl_jni PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
    
    if(MSVC AND NOT CMAKE_VS_PLATFORM_TOOLSET STREQUAL "ClangCL")
        message(WARNING "MSVC detected but ClangCL toolset not specified. "
                       "For better C11 support, consider using: "
                       "cmake -G \"Visual Studio 17 2022\" -T ClangCL")
    endif()
elseif(APPLE)
    set_target_properties(openzl_jni PROPERTIES
        SUFFIX ".dylib"
        PREFIX "lib"
    )
    target_compile_options(openzl_jni PRIVATE
        -fPIC
    )
else()
    set_target_properties(openzl_jni PROPERTIES
        SUFFIX ".so"
        PREFIX "lib"
    )
    target_compile_options(openzl_jni PRIVATE
        -fPIC
    )
endif()

if(MSVC)
    target_compile_options(openzl_jni PRIVATE
        /W3
        /wd4996
        /wd2099
        /wd4204
        /wd4221
        /std:c11
        /experimental:c11atomics
        /permissive-
        /Zc:preprocessor
    )
else()
    target_compile_options(openzl_jni PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()

install(TARGETS openzl_jni
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)