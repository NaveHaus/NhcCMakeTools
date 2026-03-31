/// @file InheritanceGraphTests.cpp
/// @brief Tests for preset inheritance graph behavior.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "Condition.h"
#include "InheritanceGraph.h"

namespace {

using nhc::preset_graph::ConditionResult;
using nhc::preset_graph::EqualsCondition;
using nhc::preset_graph::InheritanceGraphState;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::PresetAvailability;
using nhc::preset_graph::PresetInheritanceGraph;
using nhc::preset_graph::PresetPayload;
using nhc::preset_graph::UnresolvedReason;

}  // namespace

SCENARIO("PresetInheritanceGraph stores added preset payloads")
{
  GIVEN("An inheritance graph")
  {
    PresetInheritanceGraph graph;

    WHEN("A preset payload is added")
    {
      const auto nodeId = graph.AddPreset(PresetPayload{
        .Name = "debug",
        .PendingInherits = {"base"},
      });

      THEN("The payload can be retrieved")
      {
        const auto& payload = graph.GetPresetPayload(nodeId);
        REQUIRE(payload.Name == "debug");
        REQUIRE(payload.PendingInherits == std::vector<std::string>{"base"});
      }
    }
  }
}

SCENARIO("Active availability is reported for true conditions")
{
  GIVEN("A preset with a true condition and hidden false")
  {
    auto context = MacroContext{};
    context.SetMacro("presetName", "default");

    PresetInheritanceGraph graph;
    const auto nodeId = graph.AddPreset(PresetPayload{
      .Name = "default",
      .ConditionAst =
        std::make_unique<EqualsCondition>("${presetName}", "default"),
    });

    WHEN("The graph resolves availability")
    {
      graph.Resolve(context);

      THEN("The preset is active")
      {
        REQUIRE(graph.GetPresetPayload(nodeId).Availability
          == PresetAvailability::Active);
      }
    }
  }
}

SCENARIO("Inheritance links resolve when target preset exists")
{
  GIVEN("A graph with child and parent presets")
  {
    auto context = MacroContext{};

    PresetInheritanceGraph graph;
    const auto childId = graph.AddPreset(PresetPayload{
      .Name = "debug",
      .PendingInherits = {"base"},
    });
    const auto baseId = graph.AddPreset(PresetPayload{.Name = "base"});

    WHEN("Inheritance links are resolved")
    {
      graph.Resolve(context);

      THEN("The pending inherit becomes an edge")
      {
        REQUIRE(graph.GetInheritedPresets(childId)
          == std::vector<PresetInheritanceGraph::NodeId>{baseId});
        REQUIRE(graph.GetPresetPayload(childId).PendingInherits.empty());
      }
    }
  }
}

SCENARIO("Unknown condition makes graph unresolved")
{
  GIVEN("A graph with a condition requiring a missing macro")
  {
    auto context = MacroContext{};

    PresetInheritanceGraph graph;
    graph.AddPreset(PresetPayload{
      .Name = "default",
      .ConditionAst = std::make_unique<EqualsCondition>("${unknown}", "x"),
    });

    WHEN("Availability is resolved")
    {
      graph.Resolve(context);

      THEN("The graph state is unresolved")
      {
        REQUIRE(graph.ComputeState() == InheritanceGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("Hidden preset availability is Hidden")
{
  GIVEN("A hidden preset")
  {
    auto context = MacroContext{};

    PresetInheritanceGraph graph;
    const auto nodeId = graph.AddPreset(PresetPayload{
      .Name = "hidden",
      .Hidden = true,
      .ConditionAst = std::make_unique<EqualsCondition>("x", "x"),
    });

    WHEN("Availability is resolved")
    {
      graph.Resolve(context);

      THEN("The preset is hidden")
      {
        REQUIRE(graph.GetPresetPayload(nodeId).Availability
          == PresetAvailability::Hidden);
      }
    }
  }
}

SCENARIO("Preset using vendor macro is disabled")
{
  GIVEN("A preset flagged as using vendor macros")
  {
    auto context = MacroContext{};

    PresetInheritanceGraph graph;
    const auto nodeId = graph.AddPreset(PresetPayload{
      .Name = "vendor",
      .UsesVendorMacro = true,
    });

    WHEN("Availability is resolved")
    {
      graph.Resolve(context);

      THEN("The preset is disabled")
      {
        REQUIRE(graph.GetPresetPayload(nodeId).Availability
          == PresetAvailability::Disabled);
      }
    }
  }
}

SCENARIO("Inheritance cycle marks affected presets unresolved")
{
  GIVEN("A graph with a cyclic inherits relationship")
  {
    auto context = MacroContext{};

    PresetInheritanceGraph graph;
    const auto firstId = graph.AddPreset(PresetPayload{
      .Name = "A",
      .PendingInherits = {"B"},
    });
    const auto secondId = graph.AddPreset(PresetPayload{
      .Name = "B",
      .PendingInherits = {"A"},
    });

    WHEN("Inheritance links are resolved")
    {
      graph.Resolve(context);

      THEN("Both presets are marked unresolved with InheritanceCycle")
      {
        REQUIRE(graph.GetPresetPayload(firstId).Reason
          == UnresolvedReason::InheritanceCycle);
        REQUIRE(graph.GetPresetPayload(secondId).Reason
          == UnresolvedReason::InheritanceCycle);
        REQUIRE(graph.ComputeState() == InheritanceGraphState::Unresolved);
      }
    }
  }
}
