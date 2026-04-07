cmake_minimum_required(VERSION 3.14)

project(netLogger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "Suppress developer warnings")

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
option(ENABLE_SANITIZERS "Enable sanitizers" ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    # exclude self assignment as error
    # add_compile_options(-Wno-error=self-assign-overloaded)
endif()

include_directories(${INCLUDE_DIR})

add_library(ipaddr INTERFACE)
target_include_directories(ipaddr INTERFACE
    ${INCLUDE_DIR}
)

add_library(filter INTERFACE)
target_link_libraries(filter INTERFACE ipaddr)

add_library(streamproc INTERFACE)
target_link_libraries(streamproc INTERFACE 
    ipaddr
    filter
)

add_executable(main
    ${SRC_DIR}/main.cpp
)
target_link_libraries(main PRIVATE 
    ipaddr
    filter
    streamproc
)

if (ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.log)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.log 
                   ${CMAKE_CURRENT_BINARY_DIR}/test.log COPYONLY)
endif()

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)