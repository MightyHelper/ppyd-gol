cmake_minimum_required(VERSION 3.28)
project(gol)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(MPI REQUIRED)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
# Add the executable
add_executable(
        gol src/main.cpp
        src/utils.hxx
        src/canvas.hxx
        src/main_par.cpp
)

target_link_libraries(gol PRIVATE MPI::MPI_CXX)
enable_testing()
add_executable(test_src src/test_src.cpp)
target_link_libraries(test_src GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_src)