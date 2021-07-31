cmake_minimum_required(VERSION 3.5)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif ()

include_directories(..)

if (DEFINED ENV{WEBDRIVERXX_ROOT})
  set(WEBDRIVERXX_ROOT $ENV{WEBDRIVERXX_ROOT})
endif ()
find_package(WEBDRIVERXX REQUIRED)
include_directories(${WEBDRIVERXX_ROOT_DIR})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
set(ExtLibs ${ExtLibs} ${CURL_LIBRARIES})

macro(add symbol file)
  add_executable(${symbol} ${file})
  target_link_libraries(${symbol} ${ExtLibs} ${BasicLibs})
endmacro()

add(GoogleSearch google_search.cpp)
