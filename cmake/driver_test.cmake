# $$COPYRIGHT$$
#
# Wrapper CMake script to execute a driver with additional functionality beyond
# add_test(). Exits with 0 if successful, 1 (or the exit code from the driver)
# on error.
#
# Parameters:
#   DRIVER_COMMAND: (required) path to the driver executable to call
#   DRIVER_ARGS: (optional) CMake list of arguments to pass to the driver
#   DRIVER_WD: (optional) path to the working directory
#   DRIVER_CLEANUP_FILES (optional) list of files to remove after the driver has been executed
#   DRIVER_COMPARE_FILES (optional) single pair of files that must hash to the same value or an error is returned
#   DRIVER_SKIP_CLEANUP_ON_ERROR (optional) do not cleanup files if an error is detected
execute_process(
  COMMAND ${DRIVER_COMMAND} ${DRIVER_ARGS}
  WORKING_DIRECTORY ${DRIVER_WD}
  RESULT_VARIABLE _exit_code
  COMMAND_ECHO STDOUT
  )

if(_exit_code EQUAL 0 AND NOT DRIVER_COMPARE_FILES STREQUAL "")
  list(LENGTH DRIVER_COMPARE_FILES _n_files)
  if(NOT _n_files EQUAL 2)
    message(FATAL_ERROR "DRIVER_COMPARE_FILES requires exactly two paths")
  endif()

  list(GET DRIVER_COMPARE_FILES 0 _left)
  list(GET DRIVER_COMPARE_FILES 1 _right)

  file(SHA512 ${_left} _left_hash)
  file(SHA512 ${_right} _right_hash)

  if(NOT _left_hash STREQUAL _right_hash)
    message(WARNING "${_left} is not the same as ${_right}")
    set(_exit_code 1)
  endif()
endif()

if(_exit_code STREQUAL "0" OR NOT DRIVER_SKIP_CLEANUP_ON_ERROR)
  foreach(f ${DRIVER_CLEANUP_FILES})
    message("rm ${f}")
    file(REMOVE ${f})
  endforeach()
endif()

cmake_language(EXIT ${_exit_code})