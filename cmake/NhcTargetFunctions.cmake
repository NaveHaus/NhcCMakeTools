# CMake functions to reduce boilerplate when setting up targets.

include_guard()

# Apply:
#
#   NHC_TARGET_PROPERTIES

# to the named target if defined. ${InTarget} must already exist as a target.
#
#   InTarget: (required) string containing the target name :type
function(nhc_target_properties_defaults InTarget)
  if(DEFINED NHC_TARGET_PROPERTIES)
    set_target_properties(${InTarget} PROPERTIES ${NHC_TARGET_PROPERTIES})
  endif()
endfunction()

# Apply:
#
#   NHC_{PUBLIC,PRIVATE}_CXX_COMPILE_{OPTIONS,DEFINITIONS}
#
# to the named target if defined. ${InTarget} must already exist as a target.
#
# Parameters:
#   InTarget: (required) string containing the target name :type
function(nhc_target_compile_defaults InTarget)
  if(DEFINED NHC_PUBLIC_CXX_OPTIONS)
    target_compile_options(${InTarget} PUBLIC ${NHC_PRIVATE_CXX_OPTIONS})
  endif()

  if(DEFINED NHC_PRIVATE_CXX_OPTIONS)
    target_compile_options(${InTarget} PRIVATE ${NHC_PRIVATE_CXX_OPTIONS})
  endif()

  if(DEFINED NHC_PRIVATE_CXX_OPTIONS_DEBUG)
    target_compile_options(${InTarget} PRIVATE $<$<CONFIG:Debug>:${NHC_PRIVATE_CXX_OPTIONS_DEBUG}>)
  endif()

  if(DEFINED NHC_PRIVATE_CXX_OPTIONS_RELEASE)
    target_compile_options(${InTarget} PRIVATE $<$<CONFIG:Release>:${NHC_PRIVATE_CXX_OPTIONS_RELEASE}>)
  endif()

  if(DEFINED NHC_PUBLIC_CXX_DEFINITIONS)
    target_compile_definitions(${InTarget} PUBLIC ${NHC_PUBLIC_CXX_DEFINITIONS})
  endif()

  if(DEFINED NHC_PRIVATE_CXX_DEFINITIONS)
    target_compile_definitions(${InTarget} PRIVATE ${NHC_PRIVATE_CXX_DEFINITIONS})
  endif()

  if(DEFINED NHC_PUBLIC_CXX_COMPILE_FEATURES)
	  target_compile_features(${InTarget} PUBLIC ${NHC_PUBLIC_CXX_COMPILE_FEATURES})
  endif()
endfunction()

function(nhc_target_executable_properties_defaults InTarget)
  if(BUILD_SHARED_LIBS AND ${NHC_ENABLE_RELATIVE_TEST_PATHS})
    # Use relative RPATHs in the build tree (will only take affect on platforms that
    # support it):
    set_target_properties(${InTarget} PROPERTIES BUILD_RPATH_USE_ORIGIN TRUE)
  endif()
endfunction()

# Apply:
#
#   NHC_{PUBLIC,PRIVATE}_EXE_LINK_OPTIONS
#
# to the named target if defined. ${InTarget} must already exist as a target.
#
# Parameters: InTarget: (required) string containing the target name
function(nhc_target_executable_link_defaults InTarget)
  if(DEFINED NHC_PUBLIC_EXE_LINK_OPTIONS)
      target_link_options(${InTarget} PUBLIC ${NHC_PUBLIC_EXE_LINK_OPTIONS})
  endif()

  if(DEFINED NHC_PRIVATE_EXE_LINK_OPTIONS)
      target_link_options(${InTarget} PRIVATE ${NHC_PRIVATE_EXE_LINK_OPTIONS})
  endif()
endfunction()

# Apply standard computed library properties to the named target. ${InTarget}
# must already exist as a target.
#
# This sets the OUTPUT_NAME and EXPORT_NAME properties for a library target.
#
# If ${InTarget} is hyphenated, the string preceding the hyphen will be used as
# the namespace of the export; e.g. ab-libname can be referenced as ab::libname
# by consumers of the library.
#
# Parameters:
#   InTarget: (required) string containing the target name
function(nhc_target_library_properties_defaults InTarget)
  get_target_property(_export ${InTarget} EXPORT_NAME)
  if(NOT _export)
    string(REPLACE "-" "::" _alias ${InTarget})
    set(_export "${_alias}")
  endif()
  get_target_property(_libname ${InTarget} LIBRARY_OUTPUT_NAME)
  if(NOT _libname)
    string(REPLACE "-" "_" _libname ${InTarget})
    set(_libname "$<$<BOOL:${BUILD_STATIC_RUNTIME}>:lib>${_libname}$<$<CONFIG:Debug>:-d>")
  endif()

  set_target_properties(${InTarget} PROPERTIES
    LIBRARY_OUTPUT_NAME "${_libname}"
    EXPORT_NAME "${_export}"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    )

  # Set the folder if not already defined:
  get_target_property(_folder ${InTarget} FOLDER)
  if(NOT _folder)
    set_target_properties(${InTarget} PROPERTIES FOLDER "libs")
  endif()

  if(BUILD_SHARED_LIBS AND ${NHC_ENABLE_RELATIVE_TEST_PATHS})
    # Use relative RPATHs for shared libs if requested (will only take affect on
    # platforms that support it):
    get_target_property(_type ${InTarget} TYPE)
    if(TYPE STREQUAL "SHARED_LIBRARY" OR TYPE STREQUAL "MODULE_LIBRARY")
      set_target_properties(${InTarget} PROPERTIES BUILD_RPATH_USE_ORIGIN TRUE)
    endif()
  endif()
endfunction()

# Apply standard configuration to a library. ${InTarget} must already exist as a
# target.
#
# This is equivalent to:
#
#   nhc_target_properties_defaults(${InTarget})
#   nhc_target_compile_defaults(${InTarget})
#   nhc_target_library_properties_defaults(${InTarget})
#
# Parameters:
#   InTarget: (required) string containing the target name
function(nhc_configure_library InTarget)
  nhc_target_properties_defaults(${InTarget})
  nhc_target_compile_defaults(${InTarget})
  nhc_target_library_properties_defaults(${InTarget})
endfunction()

# Create a library and apply standard configuration.
#
# The target name is set to ${InTarget}. If a list of SOURCES is provided, it
# will be passed to target_sources(...).
#
# If ${InTarget} is hyphenated, the string preceding the hyphen will be used as
# the namespace of an alias target; e.g. ab-libname can be referenced as
# ab::libname by consumers of the library.
#
# Parameters:
#   InTarget: (required) string containing the target name
#
# Keywords:
#   INTERFACE, STATIC, SHARED, MODULE: (optional) the type of library to add
#   SOURCES: (optional) list of source files passed to target_sources(...)
#   USES: (optional) list of library dependencies passed to target_link_libraries(...)
#   FOLDER: (optional) path to the target in an IDE
function(nhc_add_library InTarget)
  set(flags INTERFACE STATIC SHARED MODULE)
  set(single_args FOLDER)
  set(list_args SOURCES USES)
  cmake_parse_arguments(arg "${flags}" "${single_args}" "${list_args}" ${ARGN})

  if(${arg_INTERFACE})
    set(_type INTERFACE)
  elseif(${arg_STATIC})
    set(_type STATIC)
  elseif(${arg_SHARED})
    set(_type SHARED)
  elseif(${arg_MODULE})
    set(_type MODULE)
  else()
    set(_type)
  endif()

  string(REPLACE "-" "::" _alias ${InTarget})
  add_library(${InTarget} ${_type})
  if(NOT _alias STREQUAL InTarget)
    add_library(${_alias} ALIAS ${InTarget})
  endif()

  if(DEFINED arg_SOURCES)
    target_sources(${InTarget} PRIVATE ${arg_SOURCES})
  endif()

  if(DEFINED arg_USES)
    target_link_libraries(${InTarget} ${arg_USES})
  endif()

  if(DEFINED arg_FOLDER)
    set_target_properties(${InTarget} PROPERTIES
      FOLDER "${arg_FOLDER}")
  endif()

  get_target_property(_path ${InTarget} SOURCE_DIR)
  get_target_property(_sources ${InTarget} SOURCES)
  source_group(TREE "${_path}" PREFIX "" FILES ${_sources})

  nhc_configure_library(${InTarget})
endfunction()

# Apply standard configuration to an executable. ${InTarget} must already exist
# as a target.
#
# This is equivalent to:
#
#   nhc_target_properties_defaults(${InTarget})
#   nhc_target_compile_defaults(${InTarget})
#   nhc_target_executable_link_defaults(${InTarget})
#
# Parameters:
#   InTarget: (required) string containing the target name
function(nhc_configure_executable InTarget)
  nhc_target_properties_defaults(${InTarget})
  nhc_target_compile_defaults(${InTarget})
  nhc_target_executable_properties_defaults(${InTarget})
  nhc_target_executable_link_defaults(${InTarget})

  # Help ensure DLLs are found when debugging or when running unit tests:
  if(BUILD_SHARED_LIBS)
    if(CMAKE_GENERATOR MATCHES "Visual Studio.*")
      set_target_properties(${InTarget}
        PROPERTIES VS_DEBUGGER_ENVIRONMENT
            "PATH=$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:${InTarget}>,;>"
      )
    endif()
  endif()
endfunction()

# Create an executable target and apply standard configuration.
#
# The target name is set to ${InTarget}. If a list of SOURCES is provided, it
# will be passed to target_sources(...).
#
# Parameters:
#   InTarget: (required) string containing the target name
#
# Keywords:
#   SOURCES: (optional) list of source files passed to target_sources(...)
#   USES: (optional) list of library dependencies passed to target_link_libraries(...)
#   FOLDER: (optional) path to the target in an IDE
function(nhc_add_executable InTarget)
  set(flags)
  set(single_args FOLDER)
  set(list_args SOURCES USES)
  cmake_parse_arguments(arg "${flags}" "${single_args}" "${list_args}" ${ARGN})
  add_executable(${InTarget})

  if(DEFINED arg_SOURCES)
    target_sources(${InTarget} PRIVATE ${arg_SOURCES})
  endif()

  if(DEFINED arg_USES)
    target_link_libraries(${InTarget} ${arg_USES})
  endif()

  if(DEFINED arg_FOLDER)
    set_target_properties(${InTarget} PROPERTIES
      FOLDER "${arg_FOLDER}")
  endif()

  get_target_property(_path ${InTarget} SOURCE_DIR)
  get_target_property(_sources ${InTarget} SOURCES)
  source_group(TREE "${_path}" PREFIX "" FILES ${_sources})

  nhc_configure_executable(${InTarget})
endfunction()

# Apply standard configuration to a CTest test executable. ${InTarget} must
# already exist as a target.
#
# The test executables will be output according to the paths set in:
#
#   NHC_TEST_RUNTIME_OUTPUT_DIRECTORY_DEBUG
#   NHC_TEST_RUNTIME_OUTPUT_DIRECTORY_RELEASE
#   NHC_ENABLE_RELATIVE_TEST_PATHS (if TRUE, paths will be relative to CMAKE_BINARY_DIR)
#
# Parameters:
#   InTarget: (required) string containing the target name
#
# Keywords:
#   GROUP: (optional) set the test group name, helpful with "ctest -R"
function(nhc_configure_test_executable InTarget)
  set(flags)
  set(single_args GROUP)
  set(list_args)
  cmake_parse_arguments(arg "${flags}" "${single_args}" "${list_args}" ${ARGN})

  set_target_properties(${InTarget} PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NHC_TEST_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
     RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NHC_TEST_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
  )

  if(DEFINED arg_GROUP)
    set(_test_name "${arg_GROUP}::${InTarget}")
  else()
    set(_test_name "${InTarget}")
  endif()

  if(${NHC_ENABLE_RELATIVE_TEST_PATHS})
    # Make the path relative with generator expressions:
    add_test(NAME ${_test_name}
      COMMAND $<PATH:RELATIVE_PATH,$<TARGET_FILE:${InTarget}>,${CMAKE_BINARY_DIR}>
      WORKING_DIRECTORY "./"
      CONFIGURATIONS Debug Release
    )

    # Use relative RPATHs in the build tree (will only take affect on platforms that
    # support it):
    set_target_properties(${_test_name} PROPERTIES BUILD_RPATH_USE_ORIGIN TRUE)
  else()
    add_test(NAME ${_test_name} COMMAND ${InTarget})
  endif()

  # Make sure CTest runs correctly when building DLLs:
  # See https://stackoverflow.com/a/78003965
  if(BUILD_SHARED_LIBS)
    set_tests_properties(${_test_name}
        PROPERTIES ENVIRONMENT_MODIFICATION
          "PATH=path_list_prepend:$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:${InTarget}>,\;>"
    )
  endif()
endfunction()

# Create and configure an executable target as a CMake test.
#
# This is equivalent to:
#
#  nhc_add_executable(${InTarget} ...)
#  nhc_configure_test_executable(${InTarget} ...)
#
# Parameters:
#   InTarget: (required) string containing the target name
#
# Keywords:
#   SOURCES: (optional) list of source files passed to target_sources(...)
#   USES: (optional) list of library dependencies passed to target_link_libraries(...)
#   FOLDER: (optional) path to the target in an IDE
#   GROUP: (optional) set the test group name, helpful with "ctest -R"
function(nhc_add_test_executable InTarget)
  set(flags)
  set(single_args FOLDER GROUP)
  set(list_args USES SOURCES)
  cmake_parse_arguments(arg "${flags}" "${single_args}" "${list_args}" ${ARGN})

  set(_argn)
  if(DEFINED arg_USES)
    list(APPEND _argn USES ${arg_USES})
  endif()
  if(DEFINED arg_SOURCES)
    list(APPEND _argn SOURCES ${arg_SOURCES})
  endif()
  if(DEFINED arg_FOLDER)
    list(APPEND _argn FOLDER ${arg_FOLDER})
  endif()

  nhc_add_executable(${InTarget} ${_argn})

  if(DEFINED arg_GROUP)
    set(_argn GROUP ${arg_GROUP})
  endif()
  nhc_configure_test_executable(${InTarget} ${_argn})
endfunction()

# Apply standard configuration to a CTest driver test. ${DRIVER} must
# already exist as an executable target.
#
# Keywords:
#   NAME: (required) the test name
#   DRIVER: (required) the driver target
#   ARGUMENTS (optional) list of arguments to pass to the driver
#   WORKING_DIRECTORY: (optional) the working directory for executing the driver
#   GROUP: (optional) set the test group name, helpful with "ctest -R"
#   WILL_FAIL: (optional) expect the test to fail
#   CLEANUP_FILES (optional) list of files to remove after the driver has been executed
#   COMPARE_FILES (optional) single pair of files that must hash to the same value or the test fails
#   SKIP_CLEANUP_ON_ERROR (optional) do not cleanup files if an error is detected
function(nhc_add_driver_test)
  set(flags WILL_FAIL SKIP_CLEANUP_ON_ERROR)
  set(single_args NAME DRIVER GROUP WORKING_DIRECTORY)
  set(list_args ARGUMENTS CLEANUP_FILES COMPARE_FILES)
  cmake_parse_arguments(arg "${flags}" "${single_args}" "${list_args}" ${ARGN})

  if(DEFINED arg_NAME)
    set(_name ${arg_NAME})
  else()
    message(FATAL_ERROR "NAME is required")
  endif()

  if(DEFINED arg_GROUP)
    set(_test_name "${arg_GROUP}::${_name}")
  else()
    set(_test_name "${_name}")
  endif()

  if(DEFINED arg_WORKING_DIRECTORY)
    set(_wd ${arg_WORKING_DIRECTORY})
  else()
    set(_wd "./")
  endif()

  if(DEFINED arg_ARGUMENTS)
    set(_args ${arg_ARGUMENTS})
  endif()

  if(DEFINED arg_COMPARE_FILES)
    list(LENGTH arg_COMPARE_FILES _n_files)
    if(NOT _n_files EQUAL 2)
      message(FATAL_ERROR "COMPARE_FILES requires exactly two paths")
    endif()
  endif()

  if(${NHC_ENABLE_RELATIVE_TEST_PATHS})
    set(_driver_command $<PATH:RELATIVE_PATH,$<TARGET_FILE:${arg_DRIVER}>,${CMAKE_BINARY_DIR}>)
  else()
    set(_driver_command $<TARGET_FILE:${arg_DRIVER}>)
  endif()

  add_test(
    NAME ${_test_name}
    WORKING_DIRECTORY "./"
    COMMAND
     ${CMAKE_COMMAND}
     "-DDRIVER_COMMAND=${_driver_command}"
     "-DDRIVER_ARGS=${_args}"
     "-DDRIVER_WD=${_wd}"
     "-DDRIVER_CLEANUP_FILES=${arg_CLEANUP_FILES}"
     "-DDRIVER_COMPARE_FILES=${arg_COMPARE_FILES}"
     "-DDRIVER_SKIP_CLEANUP_ON_ERROR=${arg_SKIP_CLEANUP_ON_ERROR}"
      -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/driver_test.cmake
  )

  # Expect failure if requested:
  if(arg_WILL_FAIL)
    set_tests_properties(${_test_name} PROPERTIES WILL_FAIL TRUE)
  endif()

  # Make sure CTest runs correctly when building DLLs:
  # See https://stackoverflow.com/a/78003965
  if(BUILD_SHARED_LIBS)
    set_tests_properties(${_test_name}
        PROPERTIES ENVIRONMENT_MODIFICATION
          "PATH=path_list_prepend:$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:${arg_DRIVER}>,\;>"
    )
  endif()
endfunction()