find_path(PHMAP_ROOT_DIR
  NAMES parallel_hashmap/phmap.h
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PHMAP
  FOUND_VAR PHMAP_FOUND
  REQUIRED_VARS PHMAP_ROOT_DIR
)

mark_as_advanced(PHMAP_ROOT_DIR)

if (PHMAP_FOUND)
  message(STATUS "Found valid PHMAP version:")
  message(STATUS "  PHMAP root dir: ${PHMAP_ROOT_DIR}")
endif ()
