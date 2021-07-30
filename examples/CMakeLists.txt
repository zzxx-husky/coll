cmake_minimum_required(VERSION 3.5)

include_directories(..)

macro(add symbol file)
  add_executable(${symbol} ${file})
  target_link_libraries(${symbol} ${ExtLibs} ${BasicLibs})
endmacro()

add(HelloWorld hello_world.cpp)
add(HelloWorldNative hello_world_native.cpp)
add(Calendar calendar.cpp)
add(LambdaGenerator lambda_generator.cpp)
add(TopkFreqWords topk_freq_words.cpp)
add(SortUnique sort_unique.cpp)
