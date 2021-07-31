find_path(ZMQ_ROOT_DIR
  NAMES include/zmq.hpp
  PATHS ${ZMQ_ROOT_DIR}
)

find_path(ZMQ_INCLUDE_DIR
  NAMES zmq.hpp
  PATHS ${ZMQ_ROOT_DIR}/include
  NO_DEFAULT_PATH
)

find_library(ZMQ_LIBRARY
  NAMES zmq
  PATHS ${ZMQ_ROOT_DIR}/lib
  NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZMQ
  FOUND_VAR ZMQ_FOUND
  REQUIRED_VARS ZMQ_LIBRARY ZMQ_INCLUDE_DIR
)

mark_as_advanced(ZMQ_ROOT_DIR
  ZMQ_LIBRARY
  ZMQ_INCLUDE_DIR
)

if (ZMQ_FOUND)
  message(STATUS "Found valid ZMQ version:")
  message(STATUS "  ZMQ root dir: ${ZMQ_ROOT_DIR}")
  message(STATUS "  ZMQ include dir: ${ZMQ_INCLUDE_DIR}")
  message(STATUS "  ZMQ libraries: ${ZMQ_LIBRARY}")
endif ()
