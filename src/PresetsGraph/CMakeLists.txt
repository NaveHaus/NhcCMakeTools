find_package(nlohmann_json CONFIG REQUIRED)

nhc_add_library(NhcPresetGraph STATIC
  SOURCES
    Condition.cpp
    Condition.h
    FileLoader.cpp
    FileLoader.h
    Graph.h
    InheritanceGraph.cpp
    InheritanceGraph.h
    IncludeGraph.cpp
    IncludeGraph.h
    MacroContext.cpp
    MacroContext.h
    PresetModel.cpp
    PresetModel.h
    PresetsGraph.cpp
    PresetsGraph.h
  USES
    nlohmann_json::nlohmann_json
  FOLDER "Libaries"
)

target_include_directories(NhcPresetGraph PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
