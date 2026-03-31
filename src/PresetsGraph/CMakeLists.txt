find_package(nlohmann_json CONFIG REQUIRED)

nhc_add_library(NhcPresetGraph STATIC
  SOURCES
    MacroContext.cpp
    MacroContext.h
  USES
    nlohmann_json::nlohmann_json
  FOLDER "Libaries"
)

target_include_directories(NhcPresetGraph PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
