/// @file IncludeGraphTests.cpp
/// @brief Tests for preset include graph behavior.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "IncludeGraph.h"

namespace {

using nhc::preset_graph::FilePayload;
using nhc::preset_graph::IncludeGraphState;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::PresetIncludeGraph;
using nhc::preset_graph::UnresolvedReason;

std::string
Abs(const std::string& path)
{
  return std::filesystem::absolute(path).lexically_normal().generic_string();
}

}  // namespace

SCENARIO("PresetIncludeGraph stores added file payloads")
{
  GIVEN("An include graph")
  {
    PresetIncludeGraph graph;

    WHEN("A file payload is added")
    {
      const auto nodeId = graph.AddFile(FilePayload{
        .FilePath = "CMakePresets.json",
        .PresetFileVersion = 10U,
        .PendingIncludes = {"d/e/linux-presets.json"},
      });

      THEN("The payload can be retrieved by node id")
      {
        const auto& payload = graph.GetFilePayload(nodeId);
        REQUIRE(payload.FilePath == Abs("CMakePresets.json"));
        REQUIRE(payload.PresetFileVersion == 10U);
        REQUIRE(payload.PendingIncludes
          == std::vector<std::string>{"d/e/linux-presets.json"});
      }
    }
  }
}

SCENARIO("PresetIncludeGraph reports unresolved state for pending includes")
{
  GIVEN("A graph with one file that has a pending include")
  {
    PresetIncludeGraph graph;
    graph.AddFile(FilePayload{
      .FilePath = "CMakePresets.json",
      .PendingIncludes = {"${unknown}/extra.json"},
    });

    WHEN("State is computed")
    {
      const auto state = graph.ComputeState();

      THEN("The graph is unresolved")
      {
        REQUIRE(state == IncludeGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("PresetIncludeGraph resolves include to an existing file node")
{
  GIVEN("A graph with an includer and include target")
  {
    PresetIncludeGraph graph;
    const auto includerId = graph.AddFile(FilePayload{
      .FilePath = "a/b/CMakePresets.json",
      .PendingIncludes = {"d/e/linux-presets.json"},
    });
    const auto targetId = graph.AddFile(FilePayload{
      .FilePath = "a/b/d/e/linux-presets.json",
    });
    auto context = MacroContext{};

    WHEN("Pending includes are resolved")
    {
      graph.ResolveIncludes(context);

      THEN("The include edge is added and pending include removed")
      {
        REQUIRE(graph.GetIncludedFiles(includerId)
          == std::vector<PresetIncludeGraph::NodeId>{targetId});
        REQUIRE(graph.GetFilePayload(includerId).PendingIncludes.empty());
      }
    }
  }
}

SCENARIO("PresetIncludeGraph tracks unresolved reason per file")
{
  GIVEN("A graph with one file")
  {
    PresetIncludeGraph graph;
    const auto nodeId = graph.AddFile(FilePayload{.FilePath = "missing.json"});

    WHEN("The file is marked as unresolved")
    {
      graph.MarkFileUnresolved(nodeId, UnresolvedReason::FileDoesNotExist);

      THEN("The unresolved reason is stored")
      {
        const auto& payload = graph.GetFilePayload(nodeId);
        REQUIRE(payload.IsUnresolved);
        REQUIRE(payload.Reason == UnresolvedReason::FileDoesNotExist);
      }
    }
  }
}

SCENARIO("$env include syntax is rejected")
{
  GIVEN("A graph with an include using $env")
  {
    PresetIncludeGraph graph;
    const auto nodeId = graph.AddFile(FilePayload{
      .FilePath = "CMakePresets.json",
      .PendingIncludes = {"$env{HOME}/extra.json"},
    });
    auto context = MacroContext{};

    WHEN("Pending includes are resolved")
    {
      graph.ResolveIncludes(context);

      THEN("The file is marked unresolved with UnsupportedMacro")
      {
        REQUIRE(graph.GetFilePayload(nodeId).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}

SCENARIO("preset-specific include macros are rejected")
{
  GIVEN("A graph with an include using ${presetName}")
  {
    PresetIncludeGraph graph;
    const auto nodeId = graph.AddFile(FilePayload{
      .FilePath = "CMakePresets.json",
      .PendingIncludes = {"${presetName}/extra.json"},
    });
    auto context = MacroContext{};
    context.SetMacro("presetName", "debug");

    WHEN("Pending includes are resolved")
    {
      graph.ResolveIncludes(context);

      THEN("The file is marked unresolved with UnsupportedMacro")
      {
        REQUIRE(graph.GetFilePayload(nodeId).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}

SCENARIO("Unknown ${...} include macro remains unresolved")
{
  GIVEN("A graph with an include using an unknown macro")
  {
    PresetIncludeGraph graph;
    const auto nodeId = graph.AddFile(FilePayload{
      .FilePath = "CMakePresets.json",
      .PendingIncludes = {"${unknown}/extra.json"},
    });
    auto context = MacroContext{};

    WHEN("Pending includes are resolved")
    {
      graph.ResolveIncludes(context);

      THEN("The file is marked unresolved with MissingMacro")
      {
        REQUIRE(graph.GetFilePayload(nodeId).Reason
          == UnresolvedReason::MissingMacro);
      }
    }
  }
}

SCENARIO("Version 7 and 8 reject ${...} includes")
{
  GIVEN("A graph with a version 7 file using ${...} in include")
  {
    PresetIncludeGraph graph;
    const auto nodeId = graph.AddFile(FilePayload{
      .FilePath = "CMakePresets.json",
      .PresetFileVersion = 7U,
      .PendingIncludes = {"${fileDir}/extra.json"},
    });
    auto context = MacroContext{};
    context.SetMacro("fileDir", ".");

    WHEN("Pending includes are resolved")
    {
      graph.ResolveIncludes(context);

      THEN("The file is marked unresolved with UnsupportedMacro")
      {
        REQUIRE(graph.GetFilePayload(nodeId).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}
