cmake_minimum_required(VERSION 3.28)
project(gol)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(MPI REQUIRED)
# Add the executable
add_executable(gol src/main.cpp
        src/utils.h
        src/canvas.cpp
        src/canvas.h
)
target_link_libraries(gol PRIVATE MPI::MPI_CXX)
