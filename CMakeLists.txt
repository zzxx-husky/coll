cmake_minimum_required(VERSION 3.5)
project(cpp-collection)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(ENABLE_TEST true)
set(ENABLE_PHMAP true)
set(BasicLibs pthread rt)

if (DEFINED ENV{ZMQ_ROOT})
  set(ZMQ_ROOT_DIR $ENV{ZMQ_ROOT})
endif ()
find_package(ZMQ REQUIRED)
include_directories(${ZMQ_INCLUDE_DIR})
set(BasicLibs ${BasicLibs} ${ZMQ_LIBRARY})

# http://rpg.ifi.uzh.ch/docs/glog.html
if (DEFINED ENV{GLOG_ROOT})
  set(GLOG_ROOT $ENV{GLOG_ROOT})
endif ()
find_package(GLOG REQUIRED)
include_directories(${GLOG_INCLUDE_DIR})
set(BasicLibs ${BasicLibs} ${GLOG_LIBRARY})

if (${ENABLE_PHMAP})
  add_definitions(-DENABLE_PHMAP=1)

  if (DEFINED ENV{PHMAP_ROOT})
    set(PHMAP_ROOT $ENV{PHMAP_ROOT})
  endif ()
  find_package(PHMAP)
  include_directories(${PHMAP_ROOT_DIR})
endif ()

if (DEFINED ENV{ZAF_ROOT})
  set(ZAF_ROOT_DIR $ENV{ZAF_ROOT})
endif ()
find_package(ZAF REQUIRED)
include_directories(${ZAF_INCLUDE_DIR})
set(ExtLibs ${ExtLibs} ${ZAF_LIBRARY})

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
