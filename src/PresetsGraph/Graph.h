/// @file Graph.h
/// @brief Defines a minimal directed graph abstraction.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace nhc::preset_graph {

template<typename TPayload> class Graph
{
  public:
  using NodeId = unsigned;

  NodeId AddNode(TPayload payload)
  {
    const auto nodeId = static_cast<NodeId>(m_Nodes.size());
    m_Nodes.push_back(NodeRecord{.Payload = std::move(payload)});
    return nodeId;
  }

  const TPayload& GetNodePayload(NodeId nodeId) const
  {
    return m_Nodes.at(nodeId).Payload;
  }

  TPayload& GetNodePayload(NodeId nodeId) { return m_Nodes.at(nodeId).Payload; }

  void AddEdge(NodeId sourceNodeId, NodeId destinationNodeId)
  {
    auto& source = m_Nodes.at(sourceNodeId);
    auto& destination = m_Nodes.at(destinationNodeId);

    if(std::find(source.OutgoingEdges.begin(), source.OutgoingEdges.end(),
         destinationNodeId)
      == source.OutgoingEdges.end())
    {
      source.OutgoingEdges.push_back(destinationNodeId);
    }

    if(std::find(destination.IncomingEdges.begin(),
         destination.IncomingEdges.end(), sourceNodeId)
      == destination.IncomingEdges.end())
    {
      destination.IncomingEdges.push_back(sourceNodeId);
    }
  }

  const std::vector<NodeId>& GetOutgoingEdges(NodeId nodeId) const
  {
    return m_Nodes.at(nodeId).OutgoingEdges;
  }

  const std::vector<NodeId>& GetIncomingEdges(NodeId nodeId) const
  {
    return m_Nodes.at(nodeId).IncomingEdges;
  }

  size_t NodeCount() const { return m_Nodes.size(); }

  bool HasCycle() const
  {
    constexpr std::uint8_t unvisited = 0;

    std::vector<std::uint8_t> visitStates(m_Nodes.size(), unvisited);
    for(NodeId nodeId = 0; nodeId < m_Nodes.size(); ++nodeId) {
      if(visitStates[nodeId] != unvisited) {
        continue;
      }

      if(HasCycleFrom(nodeId, visitStates)) {
        return true;
      }
    }

    return false;
  }

  private:
  struct NodeRecord
  {
    TPayload Payload;
    std::vector<NodeId> OutgoingEdges;
    std::vector<NodeId> IncomingEdges;
  };

  bool HasCycleFrom(NodeId nodeId, std::vector<std::uint8_t>& visitStates) const
  {
    constexpr std::uint8_t visiting = 1;
    constexpr std::uint8_t visited = 2;

    if(visitStates[nodeId] == visiting) {
      return true;
    }
    if(visitStates[nodeId] == visited) {
      return false;
    }

    visitStates[nodeId] = visiting;
    for(const auto childNodeId : m_Nodes[nodeId].OutgoingEdges) {
      if(HasCycleFrom(childNodeId, visitStates)) {
        return true;
      }
    }

    visitStates[nodeId] = visited;
    return false;
  }

  std::vector<NodeRecord> m_Nodes;
};

}  // namespace nhc::preset_graph
