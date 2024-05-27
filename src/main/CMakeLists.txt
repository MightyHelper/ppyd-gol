# Sequential version
add_executable(gol src/main.cpp src/utils.cpp src/canvas.cpp)
target_link_libraries(gol ${MPI_CXX_LIBRARIES})
target_include_directories(gol PRIVATE ${MPI_INCLUDE_PATH})

# Parallel version
add_executable(par_gol src/main_par.cpp src/utils.cpp src/canvas.cpp include/mpi_canvas.hxx src/vec2.cpp src/mpi_utils.cpp)
target_link_libraries(par_gol ${MPI_CXX_LIBRARIES})
target_include_directories(par_gol PRIVATE ${MPI_INCLUDE_PATH})


