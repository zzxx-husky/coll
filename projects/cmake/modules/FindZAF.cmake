find_path(ZAF_ROOT_DIR
  NAMES include/zaf/zaf.hpp
)

find_path(ZAF_INCLUDE_DIR
  NAMES zaf/zaf.hpp
  HINTS ${ZAF_ROOT_DIR}/include
)

set(HINT_DIR ${ZAF_ROOT_DIR}/lib)

find_library(ZAF_LIBRARY
  NAMES zaf libzaf
  HINTS ${HINT_DIR}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZAF
  FOUND_VAR ZAF_FOUND
  REQUIRED_VARS ZAF_LIBRARY ZAF_INCLUDE_DIR
)

mark_as_advanced(ZAF_ROOT_DIR
  ZAF_LIBRARY
  ZAF_INCLUDE_DIR
)

if (ZAF_FOUND)
  message(STATUS "Found valid ZAF version:")
  message(STATUS "  ZAF root dir: ${ZAF_ROOT_DIR}")
  message(STATUS "  ZAF include dir: ${ZAF_INCLUDE_DIR}")
  message(STATUS "  ZAF libraries: ${ZAF_LIBRARY}")
endif ()
