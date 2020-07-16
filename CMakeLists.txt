cmake_minimum_required(VERSION 3.5)
project(cpp-collection)

set(CMAKE_CXX_STANDARD 17)
#set(ExtLibs pthread rt)

include_directories(.)

macro(add symbol file)
  add_executable(${symbol} ${file})
  target_link_libraries(${symbol} ${ExtLibs})
endmacro()

add(HelloWorld examples/hello_world.cpp)
add(HelloWorldNative examples/hello_world_native.cpp)
add(Container examples/container.cpp)
add(Distinct examples/distinct.cpp)
add(Sinks examples/sinks.cpp)
add(Sort examples/sort.cpp)
add(InitTail examples/init_tail.cpp)
add(Window examples/window.cpp)
add(TopkFreqWords examples/topk_freq_words.cpp)
add(Calendar examples/calendar.cpp)
add(Branch examples/branch.cpp)
