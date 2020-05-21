# cmake_minimum_required(2.0)
project(sorting)
cmake_minimum_required(VERSION 3.15)
add_executable(wanshu.out wanshu.cxx)
target_compile_options(wanshu.out PUBLIC -std=c++17 -O3 -fopenmp)
target_link_libraries(wanshu.out -lm -lomp)


add_executable(merge_array merge_array.cxx)
target_compile_options(merge_array PUBLIC -std=c++11 -O0 -g -fopenmp)
target_link_libraries(merge_array -lm -lomp -g)
