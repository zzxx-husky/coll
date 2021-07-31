find_path(WEBDRIVERXX_ROOT_DIR
  NAMES webdriverxx/webdriverxx.h
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(WEBDRIVERXX
  FOUND_VAR WEBDRIVERXX_FOUND
  REQUIRED_VARS WEBDRIVERXX_ROOT_DIR
)

mark_as_advanced(WEBDRIVERXX_ROOT_DIR)

if (WEBDRIVERXX_FOUND)
  message(STATUS "Found valid WEBDRIVERXX version:")
  message(STATUS "  WEBDRIVERXX root dir: ${WEBDRIVERXX_ROOT_DIR}")
endif ()
