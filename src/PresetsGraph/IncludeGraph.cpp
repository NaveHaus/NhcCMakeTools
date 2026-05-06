/// @file IncludeGraph.cpp
/// @brief Implements include graph behavior.

#include "IncludeGraph.h"

#include <algorithm>
#include <filesystem>
#include <unordered_map>

namespace nhc::preset_graph {
namespace {

std::string
ResolveIncludePath(const std::string& includingFilePath,
  const std::string& includePath)
{
  auto include = std::filesystem::path(includePath);
  if(include.is_relative()) {
    const auto base = std::filesystem::path(includingFilePath).parent_path();
    include = base / include;
  }

  return include.lexically_normal().generic_string();
}

std::vector<std::string>
ExtractBraceTokens(const std::string& value)
{
  std::vector<std::string> tokens;
  size_t index = 0;
  while(index < value.size()) {
    const auto macroStart = value.find("${", index);
    if(macroStart == std::string::npos) {
      break;
    }

    const auto close = value.find('}', macroStart + 2);
    if(close == std::string::npos) {
      break;
    }

    tokens.push_back(value.substr(macroStart + 2, close - (macroStart + 2)));
    index = close + 1;
  }

  return tokens;
}

bool
ContainsUnsupportedSyntax(const std::string& includeValue)
{
  return includeValue.find("$env{") != std::string::npos
    || includeValue.find("$vendor{") != std::string::npos;
}

bool
IsSupportedBraceMacro(const std::string& macroName, unsigned presetVersion,
  const MacroContext& /*context*/)
{
  if(macroName == "presetName" || macroName == "generator") {
    return false;
  }

  if(presetVersion == 7 || presetVersion == 8) {
    return false;
  }

  if(macroName == "fileDir" || macroName == "dollar") {
    return true;
  }

  return true;
}

}  // namespace

PresetIncludeGraph::NodeId
PresetIncludeGraph::AddFile(FilePayload payload)
{
  payload.FilePath = NormalizePath(payload.FilePath);
  const auto nodeId = m_Graph.AddNode(std::move(payload));
  m_NodeByPath[m_Graph.GetNodePayload(nodeId).FilePath] = nodeId;
  return nodeId;
}

const FilePayload&
PresetIncludeGraph::GetFilePayload(NodeId nodeId) const
{
  return m_Graph.GetNodePayload(nodeId);
}

FilePayload&
PresetIncludeGraph::GetFilePayload(NodeId nodeId)
{
  return m_Graph.GetNodePayload(nodeId);
}

std::optional<PresetIncludeGraph::NodeId>
PresetIncludeGraph::FindFileNode(const std::string& filePath) const
{
  const auto normalized = NormalizePath(filePath);
  const auto it = m_NodeByPath.find(normalized);
  if(it == m_NodeByPath.end()) {
    return std::nullopt;
  }

  return it->second;
}

PresetIncludeGraph::NodeId
PresetIncludeGraph::EnsureFileNode(const std::string& filePath)
{
  if(const auto existing = FindFileNode(filePath); existing.has_value()) {
    return *existing;
  }

  return AddFile(FilePayload{.FilePath = NormalizePath(filePath)});
}

void
PresetIncludeGraph::AddIncludeEdge(NodeId sourceNodeId,
  NodeId destinationNodeId)
{
  m_Graph.AddEdge(sourceNodeId, destinationNodeId);
}

void
PresetIncludeGraph::MarkFileUnresolved(NodeId nodeId, UnresolvedReason reason)
{
  auto& payload = m_Graph.GetNodePayload(nodeId);
  payload.IsUnresolved = true;
  payload.Reason = reason;
}

const std::vector<PresetIncludeGraph::NodeId>&
PresetIncludeGraph::GetIncludedFiles(NodeId nodeId) const
{
  return m_Graph.GetOutgoingEdges(nodeId);
}

size_t
PresetIncludeGraph::FileCount() const
{
  return m_Graph.NodeCount();
}

bool
PresetIncludeGraph::HasCycle() const
{
  return m_Graph.HasCycle();
}

void
PresetIncludeGraph::ResolveIncludes(const MacroContext& context)
{
  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    const auto filePath = m_Graph.GetNodePayload(nodeId).FilePath;
    const auto version =
      m_Graph.GetNodePayload(nodeId).PresetFileVersion.value_or(10U);
    const auto pendingIncludes = m_Graph.GetNodePayload(nodeId).PendingIncludes;
    std::vector<std::string> remainingIncludes;

    for(const auto& includeValue : pendingIncludes) {
      if(ContainsUnsupportedSyntax(includeValue)) {
        MarkFileUnresolved(nodeId, UnresolvedReason::UnsupportedMacro);
        remainingIncludes.push_back(includeValue);
        continue;
      }

      const auto braceTokens = ExtractBraceTokens(includeValue);
      if(version == 7 || version == 8) {
        if(!braceTokens.empty()) {
          MarkFileUnresolved(nodeId, UnresolvedReason::UnsupportedMacro);
          remainingIncludes.push_back(includeValue);
          continue;
        }
      } else {
        if(std::any_of(braceTokens.begin(), braceTokens.end(),
             [&](const auto& macroName) {
               return !IsSupportedBraceMacro(macroName, version, context);
             }))
        {
          MarkFileUnresolved(nodeId, UnresolvedReason::UnsupportedMacro);
          remainingIncludes.push_back(includeValue);
          continue;
        }
      }

      auto localContext = context;
      localContext.SetMacro("fileDir",
        std::filesystem::path(filePath).parent_path().generic_string());
      localContext.SetMacro("dollar", "$");

      const auto expanded = localContext.ExpandString(includeValue);
      if(expanded.Status == ExpansionStatus::PartiallyExpanded) {
        MarkFileUnresolved(nodeId, UnresolvedReason::MissingMacro);
        remainingIncludes.push_back(includeValue);
        continue;
      }

      const auto resolvedPath =
        ResolveIncludePath(filePath, expanded.ExpandedString);
      const auto target = EnsureFileNode(resolvedPath);
      m_Graph.AddEdge(nodeId, target);
    }

    m_Graph.GetNodePayload(nodeId).PendingIncludes =
      std::move(remainingIncludes);
  }
}

IncludeGraphState
PresetIncludeGraph::ComputeState() const
{
  if(m_Graph.NodeCount() == 0) {
    return IncludeGraphState::Empty;
  }

  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    const auto& payload = GetFilePayload(nodeId);
    if(payload.IsUnresolved || !payload.PendingIncludes.empty()) {
      return IncludeGraphState::Unresolved;
    }
  }

  return IncludeGraphState::Resolved;
}

std::string
PresetIncludeGraph::NormalizePath(const std::string& path)
{
  return std::filesystem::absolute(std::filesystem::path(path))
    .lexically_normal()
    .generic_string();
}

}  // namespace nhc::preset_graph
