enable_testing()

# Sequential tests
add_executable(test_src
  src/test_src.cpp
  ../main/src/canvas.cpp
  ../main/src/utils.cpp
)
target_link_libraries(test_src GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_src)

# Parallel tests
add_executable(test_par_src
  src/test_par_src.cpp
  ../main/src/canvas.cpp
  ../main/src/utils.cpp
  ../main/include/mpi_canvas.hxx
  ../main/src/vec2.cpp
  ../main/src/mpi_utils.cpp
)
target_link_libraries(test_par_src GTest::gtest_main ${MPI_CXX_LIBRARIES})
target_include_directories(test_par_src PRIVATE ${MPI_INCLUDE_PATH})
include(GoogleTest)
gtest_discover_tests(test_par_src)
