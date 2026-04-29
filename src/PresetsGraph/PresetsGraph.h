/// @file PresetsGraph.h
/// @brief Defines the composite presets graph manager.

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "FileLoader.h"
#include "IncludeGraph.h"
#include "InheritanceGraph.h"
#include "PresetModel.h"

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

struct WorkflowDiagnostic
{
  std::string PresetName;
  std::string Message;
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

  const PresetModel& GetPresetModel() const;
  PresetModel& GetPresetModel();

  const std::vector<WorkflowDiagnostic>& GetWorkflowDiagnostics() const;

  PresetsGraphState ComputeState() const;

  private:
  unsigned SupportedPresetFileVersion() const;
  bool IsVersionSupported(unsigned presetVersion) const;
  bool MeetsMinimum(const CMakeVersion& required) const;
  bool TryLoadFileNode(PresetIncludeGraph::NodeId nodeId);
  void RefreshInheritanceGraph();
  void ValidateWorkflowPresets();

  const FileLoader& m_FileLoader;
  CMakeVersion m_SimulatedVersion;

  std::unordered_set<std::string> m_LoadedFiles;
  std::unordered_map<std::string, std::vector<std::string>> m_PresetNamesByFile;
  std::vector<PresetIncludeGraph::NodeId> m_RootFileIds;

  PresetIncludeGraph m_IncludeGraph;
  PresetInheritanceGraph m_InheritanceGraph;
  PresetModel m_PresetModel;
  std::vector<WorkflowDiagnostic> m_WorkflowDiagnostics;
};

}  // namespace nhc::preset_graph
