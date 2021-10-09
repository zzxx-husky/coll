find_path(ZAF_ROOT_DIR
  NAMES include/zaf/zaf.hpp
)

find_path(ZAF_INCLUDE_DIR
  NAMES zaf/zaf.hpp
  HINTS ${ZAF_ROOT_DIR}/include
)

find_file(ZAF_DEPS
  NAMES zaf_dependencies
  HINTS ${ZAF_ROOT_DIR}
)

find_library(ZAF_LIBRARY
  NAMES zaf libzaf
  HINTS ${ZAF_ROOT_DIR}/lib
)

file(STRINGS "${ZAF_DEPS}" DEPS_HEADERS_AND_LIBS)
list(GET DEPS_HEADERS_AND_LIBS 0 ZAF_DEPS_HEADERS)
list(GET DEPS_HEADERS_AND_LIBS 1 ZAF_DEPS_LIBS)
list(GET DEPS_HEADERS_AND_LIBS 2 ZAF_USE_PHMAP)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZAF
  FOUND_VAR ZAF_FOUND
  REQUIRED_VARS ZAF_LIBRARY ZAF_INCLUDE_DIR ZAF_DEPS_HEADERS ZAF_DEPS_LIBS ZAF_USE_PHMAP
)

mark_as_advanced(ZAF_ROOT_DIR
  ZAF_LIBRARY
  ZAF_INCLUDE_DIR
  ZAF_DEPS_HEADERS
  ZAF_DEPS_LIBS
  ZAF_USE_PHMAP
)

if (ZAF_FOUND)
  message(STATUS "Found valid ZAF version:")
  message(STATUS "  ZAF root dir: ${ZAF_ROOT_DIR}")
  message(STATUS "  ZAF include dir: ${ZAF_INCLUDE_DIR}")
  message(STATUS "  ZAF libraries: ${ZAF_LIBRARY}")
  message(STATUS "  ZAF dependency include dirs: ${ZAF_DEPS_HEADERS}")
  message(STATUS "  ZAF dependency library dirs: ${ZAF_DEPS_LIBS}")
  message(STATUS "  ZAF use_phmap: ${ZAF_USE_PHMAP}")
endif ()
