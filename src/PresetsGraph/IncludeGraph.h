/// @file IncludeGraph.h
/// @brief Defines the include graph for preset files.

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Graph.h"
#include "MacroContext.h"

namespace nhc::preset_graph {

enum class IncludeGraphState
{
  Empty,
  Resolved,
  Unresolved
};

enum class UnresolvedReason
{
  FileDoesNotExist,
  InvalidJson,
  MissingMacro,
  UnsupportedMacro,
  EnvironmentCycle,
  InvalidCondition,
  IncludeCycle,
  InheritanceCycle,
  CMakeMinimumRequiredNotMet,
  PresetVersionUnsupported,
  PresetVersionMissing,
  IncludeFieldUnsupportedInPresetVersion
};

/// Payload stored for a preset file node.
struct FilePayload
{
  std::string FilePath;
  std::optional<unsigned> PresetFileVersion;
  std::optional<std::string> CMakeMinimumRequired;
  std::vector<std::string> PendingIncludes;
  bool IsUnresolved = false;
  std::optional<UnresolvedReason> Reason;
};

/// Graph model for preset file includes.
class PresetIncludeGraph
{
  public:
  using NodeId = Graph<FilePayload>::NodeId;

  NodeId AddFile(FilePayload payload);

  const FilePayload& GetFilePayload(NodeId nodeId) const;

  FilePayload& GetFilePayload(NodeId nodeId);

  std::optional<NodeId> FindFileNode(const std::string& filePath) const;

  NodeId EnsureFileNode(const std::string& filePath);

  void AddIncludeEdge(NodeId sourceNodeId, NodeId destinationNodeId);

  void MarkFileUnresolved(NodeId nodeId, UnresolvedReason reason);

  const std::vector<NodeId>& GetIncludedFiles(NodeId nodeId) const;

  size_t FileCount() const;

  bool HasCycle() const;

  void ResolveIncludes(const MacroContext& context);

  IncludeGraphState ComputeState() const;

  private:
  static std::string NormalizePath(const std::string& path);

  Graph<FilePayload> m_Graph;
  std::unordered_map<std::string, NodeId> m_NodeByPath;
};

}  // namespace nhc::preset_graph
