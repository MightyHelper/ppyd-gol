cmake_minimum_required(VERSION 3.28)
project(gol)
set(CMAKE_CXX_STANDARD 20)
find_package(MPI REQUIRED)
# Add the includes
include_directories(include)
# Add the executable
add_executable(gol src/main.cpp src/utils.cpp src/golio.cpp)
target_link_libraries(gol PRIVATE MPI::MPI_CXX)
