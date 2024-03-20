# STM32 Minimal CMake project for C/C++ projects
cmake_minimum_required(VERSION 3.14)

# Set project name and some defaults
if ("${PROJECT_NAME}" STREQUAL "")
    set(PROJECT_NAME firmware-tests)
endif()
project(
    ${PROJECT_NAME}
    )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set debug mode as default
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")


# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

###############################################################################

set(This Example_tests)

set(Sources 
    main_test.cpp
    )
message(STATUS "CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES: ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES}")
message(STATUS "CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
    
enable_testing()

add_executable(
  ${This}
  ${Sources}
)

target_link_libraries(
    ${This}
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Example_tests)
