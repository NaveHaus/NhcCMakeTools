/// @file CoreGraphTests.cpp
/// @brief Tests for core graph behavior.

#include <catch2/catch_test_macros.hpp>

#include "Graph.h"

namespace {

using nhc::preset_graph::Graph;

}  // namespace

SCENARIO("Graph assigns unique sequential node IDs")
{
  GIVEN("An empty graph")
  {
    Graph<int> graph;

    WHEN("Two payload nodes are added")
    {
      const auto firstId = graph.AddNode(10);
      const auto secondId = graph.AddNode(20);

      THEN("The IDs are unique and zero-based")
      {
        REQUIRE(firstId == 0U);
        REQUIRE(secondId == 1U);
      }
    }
  }
}

SCENARIO("Graph tracks directed edges")
{
  GIVEN("A graph with two nodes")
  {
    Graph<int> graph;
    const auto firstId = graph.AddNode(10);
    const auto secondId = graph.AddNode(20);

    WHEN("A directed edge is added")
    {
      graph.AddEdge(firstId, secondId);

      THEN("Outgoing and incoming edge collections are updated")
      {
        REQUIRE(graph.GetOutgoingEdges(firstId)
          == std::vector<Graph<int>::NodeId>{secondId});
        REQUIRE(graph.GetIncomingEdges(secondId)
          == std::vector<Graph<int>::NodeId>{firstId});
      }
    }
  }
}

SCENARIO("Graph reports cycle presence")
{
  GIVEN("A graph with an acyclic chain")
  {
    Graph<int> graph;
    const auto firstId = graph.AddNode(10);
    const auto secondId = graph.AddNode(20);
    const auto thirdId = graph.AddNode(30);
    graph.AddEdge(firstId, secondId);
    graph.AddEdge(secondId, thirdId);

    THEN("No cycle is detected")
    {
      REQUIRE_FALSE(graph.HasCycle());
    }

    WHEN("A back edge creates a cycle")
    {
      graph.AddEdge(thirdId, firstId);

      THEN("Cycle detection reports the graph contains a cycle")
      {
        REQUIRE(graph.HasCycle());
      }
    }
  }
}
