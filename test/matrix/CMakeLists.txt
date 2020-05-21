# cmake_minimum_required(2.0)
project(sorting)
cmake_minimum_required(VERSION 3.0)

add_executable(wanshu.out matrix.cxx)
target_compile_options(wanshu.out PUBLIC -std=c++17 -O0 -fopenmp)
target_link_libraries(wanshu.out -lm -lomp)

add_executable(wanshusort.out matrix2.cxx)
target_compile_options(wanshusort.out PUBLIC -std=c++17 -O3 -fopenmp)
target_link_libraries(wanshusort.out -lm -lomp)
