/// @file InheritanceGraph.cpp
/// @brief Implements inheritance graph behavior.

#include "InheritanceGraph.h"

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace nhc::preset_graph {

PresetInheritanceGraph::NodeId
PresetInheritanceGraph::AddPreset(PresetPayload payload)
{
  return m_Graph.AddNode(std::move(payload));
}

const PresetPayload&
PresetInheritanceGraph::GetPresetPayload(NodeId nodeId) const
{
  return m_Graph.GetNodePayload(nodeId);
}

const std::vector<PresetInheritanceGraph::NodeId>&
PresetInheritanceGraph::GetInheritedPresets(NodeId nodeId) const
{
  return m_Graph.GetOutgoingEdges(nodeId);
}

void
PresetInheritanceGraph::Resolve(const MacroContext& context)
{
  ResolveInheritanceLinks();
  EvaluateAvailability(context);
  DetectCycles();
}

InheritanceGraphState
PresetInheritanceGraph::ComputeState() const
{
  if(m_Graph.NodeCount() == 0) {
    return InheritanceGraphState::Empty;
  }

  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    const auto& payload = GetPresetPayload(nodeId);
    if(payload.IsUnresolved || !payload.PendingInherits.empty()
      || payload.Availability == PresetAvailability::Unknown)
    {
      return InheritanceGraphState::Unresolved;
    }
  }

  return InheritanceGraphState::Resolved;
}

void
PresetInheritanceGraph::ResolveInheritanceLinks()
{
  std::unordered_map<std::string, NodeId> nodeByName;
  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    nodeByName.emplace(GetPresetPayload(nodeId).Name, nodeId);
  }

  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    auto& payload = m_Graph.GetNodePayload(nodeId);
    std::vector<std::string> remaining;
    for(const auto& inheritedName : payload.PendingInherits) {
      const auto target = nodeByName.find(inheritedName);
      if(target == nodeByName.end()) {
        remaining.push_back(inheritedName);
        continue;
      }

      m_Graph.AddEdge(nodeId, target->second);
    }

    payload.PendingInherits = std::move(remaining);
  }
}

void
PresetInheritanceGraph::EvaluateAvailability(const MacroContext& context)
{
  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    auto& payload = m_Graph.GetNodePayload(nodeId);

    if(payload.Hidden) {
      payload.Availability = PresetAvailability::Hidden;
      continue;
    }

    if(payload.UsesVendorMacro) {
      payload.Availability = PresetAvailability::Disabled;
      continue;
    }

    if(!payload.ConditionAst) {
      payload.Availability = PresetAvailability::Active;
      continue;
    }

    const auto result = payload.ConditionAst->Evaluate(context);
    switch(result) {
      case ConditionResult::True:
        payload.Availability = PresetAvailability::Active;
        break;
      case ConditionResult::False:
        payload.Availability = PresetAvailability::Disabled;
        break;
      case ConditionResult::Unknown:
        payload.Availability = PresetAvailability::Unknown;
        break;
    }
  }
}

void
PresetInheritanceGraph::DetectCycles()
{
  constexpr std::uint8_t unvisited = 0;
  constexpr std::uint8_t visiting = 1;
  constexpr std::uint8_t visited = 2;

  std::vector<std::uint8_t> visitState(m_Graph.NodeCount(), unvisited);
  std::vector<NodeId> stack;

  const auto dfs = [&](const auto& self, NodeId nodeId) -> void {
    visitState[nodeId] = visiting;
    stack.push_back(nodeId);

    for(const auto child : m_Graph.GetOutgoingEdges(nodeId)) {
      if(visitState[child] == unvisited) {
        self(self, child);
        continue;
      }

      if(visitState[child] == visiting) {
        const auto cycleStart = std::find(stack.begin(), stack.end(), child);
        for(auto it = cycleStart; it != stack.end(); ++it) {
          MarkUnresolved(*it, UnresolvedReason::InheritanceCycle);
        }
      }
    }

    stack.pop_back();
    visitState[nodeId] = visited;
  };

  for(NodeId nodeId = 0; nodeId < m_Graph.NodeCount(); ++nodeId) {
    if(visitState[nodeId] == unvisited) {
      dfs(dfs, nodeId);
    }
  }
}

void
PresetInheritanceGraph::MarkUnresolved(NodeId nodeId, UnresolvedReason reason)
{
  auto& payload = m_Graph.GetNodePayload(nodeId);
  payload.IsUnresolved = true;
  payload.Reason = reason;
}

}  // namespace nhc::preset_graph
