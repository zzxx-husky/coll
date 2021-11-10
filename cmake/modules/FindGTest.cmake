find_path(GTEST_ROOT
  NAMES include/gtest/gtest.h
)

find_path(GTEST_INCLUDE_DIR
  NAMES gtest/gtest.h
  HINTS ${GTEST_ROOT}/include
)

find_library(GTEST_LIBRARY
  NAMES gtest
  HINTS ${GTEST_ROOT}/lib
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GTest
  FOUND_VAR GTest_FOUND
  REQUIRED_VARS GTEST_LIBRARY GTEST_INCLUDE_DIR
)

mark_as_advanced(GTEST_ROOT
  GTEST_LIBRARY
  GTEST_INCLUDE_DIR
)

if (GTest_FOUND)
  message(STATUS "Found valid GTest version:")
  message(STATUS "  GTest root dir: ${GTEST_ROOT}")
  message(STATUS "  GTest include dir: ${GTEST_INCLUDE_DIR}")
  message(STATUS "  GTest libraries: ${GTEST_LIBRARY}")
endif ()
