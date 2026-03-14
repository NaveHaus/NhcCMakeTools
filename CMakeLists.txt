cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(NhcCMakeTools LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_COMPILER_ID)
  message(FATAL_ERROR "${PROJECT_NAME} requires a C++ compiler")
endif()

set(NHC_CXX_STANDARD cxx_std_23)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMakeDependentOption)
include(NhcCompiler${CMAKE_CXX_COMPILER_ID})
include(NhcDefaultPaths)
include(NhcTargetFunctions)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
endif()

if(NOT DEFINED BUILD_STATIC_RUNTIME)
  cmake_dependent_option(BUILD_STATIC_RUNTIME
    "Build against a static runtime" ON
    "NOT BUILD_SHARED_LIBS" OFF
  )
endif()

# Only enable testing by default for standalone builds:
cmake_dependent_option(BUILD_TESTING
  "Eenable testing"
  ON  # Show and default to ON if...
  "PROJECT_IS_TOP_LEVEL"
  OFF # Oherwise, default to OFF.
)

find_package(Qt6 REQUIRED COMPONENTS Base)

add_subdirectory(src)

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()