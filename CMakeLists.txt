cmake_minimum_required(VERSION 3.5)
project(cpp-collection)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(ENABLE_TEST true)
set(BasicLibs pthread rt)

if (DEFINED ENV{ZAF_ROOT})
  set(ZAF_ROOT_DIR $ENV{ZAF_ROOT})
endif ()
find_package(ZAF REQUIRED)
include_directories(${ZAF_INCLUDE_DIR} ${ZAF_DEPS_HEADERS})
set(ExtLibs ${ExtLibs} ${ZAF_LIBRARY} ${ZAF_DEPS_LIBS})
if (${ZAF_USE_PHMAP})
  add_definitions(-DENABLE_PHMAP=1)
endif()

if (${ENABLE_TEST})
  if (DEFINED ENV{GTEST_ROOT})
    set(GTEST_ROOT $ENV{GTEST_ROOT})
  endif ()

  find_package(GTest)
  include_directories(${GTEST_INCLUDE_DIR})
  set(TestLibs ${GTEST_LIBRARY})

  add_subdirectory(tests)
endif ()

add_subdirectory(benchmarks)
add_subdirectory(examples)
