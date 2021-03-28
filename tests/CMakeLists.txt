cmake_minimum_required(VERSION 3.5)

include_directories(..)

file(GLOB_RECURSE TestsSrc *.cpp)
add_executable(Tests ${TestsSrc})
target_link_libraries(Tests ${TestLibs} ${ExtLibs} ${BasicLibs})
