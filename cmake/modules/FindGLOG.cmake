find_path(GLOG_ROOT
  NAMES include/glog/logging.h
)

find_path(GLOG_INCLUDE_DIR
  NAMES glog/logging.h
  HINTS ${GLOG_ROOT}/include
)

set(HINT_DIR ${GLOG_ROOT}/lib)

find_library(GLOG_LIBRARY
  NAMES glog libglog
  HINTS ${HINT_DIR}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GLOG
  FOUND_VAR GLOG_FOUND
  REQUIRED_VARS GLOG_LIBRARY GLOG_INCLUDE_DIR
)

mark_as_advanced(GLOG_ROOT
  GLOG_LIBRARY
  GLOG_INCLUDE_DIR
)

if (GLOG_FOUND)
  message(STATUS "Found valid GLOG version:")
  message(STATUS "  GLOG root dir: ${GLOG_ROOT}")
  message(STATUS "  GLOG include dir: ${GLOG_INCLUDE_DIR}")
  message(STATUS "  GLOG libraries: ${GLOG_LIBRARY}")
endif ()
