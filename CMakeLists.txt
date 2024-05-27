cmake_minimum_required(VERSION 3.28)
project(gol)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O5")

find_package(MPI REQUIRED)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src)

enable_testing()
add_executable(test_src
        src/test_src.cpp
        src/canvas.cpp
        src/utils.cpp
)
target_link_libraries(test_src GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_src)

add_executable(test_par_src
        src/test_par_src.cpp
        src/canvas.cpp
        src/utils.cpp
        src/mpi_canvas.hxx
        src/vec2.cpp
        src/mpi_utils.cpp
)
target_link_libraries(test_par_src GTest::gtest_main ${MPI_CXX_LIBRARIES})
target_include_directories(test_par_src PRIVATE ${MPI_INCLUDE_PATH})
include(GoogleTest)
gtest_discover_tests(test_par_src)