project(mpis)

set(CMAKE_CXX_COMPILER mpic++)
add_executable(cannon1 cannon1.cxx)
add_executable(cannon2 cannon2.cxx)
add_executable(mpi_test mpi_test.cxx)
target_compile_options(mpi_test PRIVATE -O0 )

add_executable(gauss gauss.cxx codetimer.cxx)
target_compile_options(gauss PRIVATE -O0 -g -std=c++14)
