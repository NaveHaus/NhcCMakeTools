/// @file PresetsGraph.h
/// @brief Defines the composite presets graph manager.

#pragma once

#include <vector>
#include <string>
#include <unordered_set>

#include "FileLoader.h"
#include "IncludeGraph.h"
#include "InheritanceGraph.h"

namespace nhc::preset_graph {

enum class PresetsGraphState
{
  Empty,
  Resolved,
  Unresolved
};

struct CMakeVersion
{
  unsigned Major = 3;
  unsigned Minor = 31;
  unsigned Patch = 0;
};

/// Coordinates include and inheritance graph resolution.
class PresetsGraph
{
  public:
  PresetsGraph(const FileLoader& fileLoader, CMakeVersion simulatedVersion);

  PresetIncludeGraph::NodeId AddRootFile(const std::string& path);

  void ApplyContext(const MacroContext& context);

  const PresetIncludeGraph& GetIncludeGraph() const;
  PresetIncludeGraph& GetIncludeGraph();

  const PresetInheritanceGraph& GetInheritanceGraph() const;
  PresetInheritanceGraph& GetInheritanceGraph();

  PresetsGraphState ComputeState() const;

  private:
  unsigned SupportedPresetFileVersion() const;
  bool IsVersionSupported(unsigned presetVersion) const;
  bool MeetsMinimum(const CMakeVersion& required) const;
  bool TryLoadFileNode(PresetIncludeGraph::NodeId nodeId);

  const FileLoader& m_FileLoader;
  CMakeVersion m_SimulatedVersion;

  std::unordered_set<std::string> m_LoadedFiles;
  std::vector<PresetIncludeGraph::NodeId> m_RootFileIds;

  PresetIncludeGraph m_IncludeGraph;
  PresetInheritanceGraph m_InheritanceGraph;
};

}  // namespace nhc::preset_graph
