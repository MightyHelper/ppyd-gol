add_executable(gol main.cpp utils.cpp canvas.cpp)
target_link_libraries(gol ${MPI_CXX_LIBRARIES})
target_include_directories(gol PRIVATE ${MPI_INCLUDE_PATH})

add_executable(par_gol main_par.cpp utils.cpp canvas.cpp mpi_canvas.hxx vec2.cpp mpi_utils.cpp)
target_link_libraries(par_gol ${MPI_CXX_LIBRARIES})
target_include_directories(par_gol PRIVATE ${MPI_INCLUDE_PATH})


