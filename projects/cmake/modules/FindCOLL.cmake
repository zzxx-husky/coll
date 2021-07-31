find_path(COLL_ROOT_DIR
  NAMES coll/coll.hpp
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(COLL
  FOUND_VAR COLL_FOUND
  REQUIRED_VARS COLL_ROOT_DIR
)

mark_as_advanced(COLL_ROOT_DIR)

if (COLL_FOUND)
  message(STATUS "Found valid COLL version:")
  message(STATUS "  COLL root dir: ${COLL_ROOT_DIR}")
endif ()
