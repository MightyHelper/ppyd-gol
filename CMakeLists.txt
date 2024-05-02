cmake_minimum_required(VERSION 3.28)
project(gol)
set(CMAKE_CXX_STANDARD 17)
find_package(MPI REQUIRED)

add_executable(gol src/main.cpp)
target_link_libraries(gol PRIVATE MPI::MPI_CXX)
