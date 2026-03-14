# MSVC compiler defaults.

include_guard()

message(STATUS "${PROJECT_NAME}: configuring for ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} (${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})")

# Added to nhc_TARGET_PROPERTIES:
if(NOT DEFINED MSVC_RUNTIME_LIBRARY)
  if(BUILD_STATIC_RUNTIME)
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif() 
endif()

# Enforce the default for all projects:
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY})
endif()

set(_nhc_target_properties
  MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY}
)

set(_nhc_public_cxx_definitions
  _UNICODE
  UNICODE
  NOMINMAX
  WIN32_LEAN_AND_MEAN
)

set(_nhc_private_cxx_options
  /permissive-   # Strict(er) standards conformance
  /GL            # Link-time code generation
)
if(PROJECT_IS_TOP_LEVEL)
  list(APPEND _nhc_private_cxx_options
    /W4          # Most warnings
  )
endif()

set(NHC_TARGET_PROPERTIES ${_nhc_target_properties})
set(NHC_PUBLIC_CXX_COMPILE_FEATURES ${NHC_CXX_STD})
set(NHC_PUBLIC_CXX_DEFINITIONS ${_nhc_public_cxx_definitions})
set(NHC_PRIVATE_CXX_DEFINITIONS ${_nhc_private_cxx_definitions})
set(NHC_PRIVATE_CXX_OPTIONS ${_nhc_private_cxx_options})
set(NHC_PRIVATE_CXX_OPTIONS_DEBUG ${_nhc_private_cxx_options_debug})
set(NHC_PRIVATE_CXX_OPTIONS_RELEASE ${_nhc_private_cxx_options_release})
set(NHC_PRIVATE_EXE_LINK_OPTIONS ${_nhc_private_exe_link_options})