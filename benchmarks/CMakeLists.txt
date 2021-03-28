cmake_minimum_required(VERSION 3.5)

include_directories(..)

macro(add symbol file)
  add_executable(${symbol} ${file})
  target_link_libraries(${symbol} ${ExtLibs} ${BasicLibs})
endmacro()

add(ParallelSort parallel_sort.cpp)
