/// @file GraphManagerTests.cpp
/// @brief Tests for PresetsGraph manager behavior.

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "PresetsGraph.h"

namespace {

using nhc::preset_graph::CMakeVersion;
using nhc::preset_graph::FileLoadResult;
using nhc::preset_graph::FileLoader;
using nhc::preset_graph::BuildPreset;
using nhc::preset_graph::ConfigurePreset;
using nhc::preset_graph::PresetAvailability;
using nhc::preset_graph::PresetInheritanceGraph;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::PresetKind;
using nhc::preset_graph::PresetPayload;
using nhc::preset_graph::PresetsGraph;
using nhc::preset_graph::PresetsGraphState;
using nhc::preset_graph::UnresolvedReason;
using nhc::preset_graph::WorkflowPreset;

std::string
Abs(const std::string& path)
{
  return std::filesystem::absolute(path).lexically_normal().generic_string();
}

template<size_t TPresetCount>
const PresetPayload&
FindPayload(const PresetInheritanceGraph& graph, std::string_view name)
{
  for(auto nodeId = PresetInheritanceGraph::NodeId{0}; nodeId < TPresetCount;
    ++nodeId)
  {
    const auto& payload = graph.GetPresetPayload(nodeId);
    if(payload.Name == name) {
      return payload;
    }
  }

  FAIL("Preset payload was not found");
  std::abort();
}

class TestFileLoader final : public FileLoader
{
  public:
  mutable unsigned LoadCount = 0;
  mutable std::unordered_map<std::string, unsigned> LoadCountsByPath;
  std::unordered_map<std::string, std::string> Files;

  FileLoadResult LoadFile(const std::string& path) const override
  {
    ++LoadCount;
    ++LoadCountsByPath[Abs(path)];
    const auto it = Files.find(path);
    if(it != Files.end()) {
      return FileLoadResult{.Success = true, .Contents = it->second};
    }

    const auto normalizedPath = Abs(path);
    for(const auto& [candidatePath, contents] : Files) {
      if(Abs(candidatePath) == normalizedPath) {
        return FileLoadResult{.Success = true, .Contents = contents};
      }
    }

    {
      return FileLoadResult{
        .Success = false,
        .FileDoesNotExist = true,
        .Contents = {},
      };
    }
  }
};

}  // namespace

SCENARIO("ApplyContext discovers new include files")
{
  GIVEN("A root file including another file")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["extra.json"]})";
    loader.Files["a/extra.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("A new file node is discovered")
      {
        REQUIRE(manager.GetIncludeGraph().FileCount() == 2U);
      }
    }
  }
}

SCENARIO("Manager loads discovered files through file loader")
{
  GIVEN("A root file with one include")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["extra.json"]})";
    loader.Files["a/extra.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The loader is invoked for discovered files")
      {
        REQUIRE(loader.LoadCount >= 2U);
      }
    }
  }
}

SCENARIO("Missing include file marks node unresolved")
{
  GIVEN("An include path that cannot be loaded")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["missing.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The missing file node is unresolved with FileDoesNotExist")
      {
        REQUIRE(manager.GetIncludeGraph().FileCount() == 2U);
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).Reason
          == UnresolvedReason::FileDoesNotExist);
        REQUIRE(manager.ComputeState() == PresetsGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("Invalid JSON marks node unresolved")
{
  GIVEN("A discovered include file containing invalid JSON")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["bad.json"]})";
    loader.Files["a/bad.json"] = "not json";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is unresolved with InvalidJson")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).Reason
          == UnresolvedReason::InvalidJson);
      }
    }
  }
}

SCENARIO("Reloading a previously missing include clears stale unresolved state")
{
  GIVEN("An include file that is missing during the first context application")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["missing.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");
    manager.ApplyContext(MacroContext{});

    REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).Reason
      == UnresolvedReason::FileDoesNotExist);

    WHEN("The file is available during a later context application")
    {
      loader.Files["a/missing.json"] = R"({"version":10})";
      manager.ApplyContext(MacroContext{});

      THEN("The stale missing-file unresolved state is cleared")
      {
        const auto& payload = manager.GetIncludeGraph().GetFilePayload(1);
        REQUIRE_FALSE(payload.IsUnresolved);
        REQUIRE_FALSE(payload.Reason.has_value());
      }
    }
  }
}

SCENARIO("Failed reload records the current unresolved reason")
{
  GIVEN(
    "An include file that has invalid JSON during the first context application")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["bad.json"]})";
    loader.Files["a/bad.json"] = "not json";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");
    manager.ApplyContext(MacroContext{});

    REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).Reason
      == UnresolvedReason::InvalidJson);

    WHEN("The file is absent during a later context application")
    {
      loader.Files.erase("a/bad.json");
      manager.ApplyContext(MacroContext{});

      THEN("The unresolved reason is assigned from the current load attempt")
      {
        const auto& payload = manager.GetIncludeGraph().GetFilePayload(1);
        REQUIRE(payload.IsUnresolved);
        REQUIRE(payload.Reason == UnresolvedReason::FileDoesNotExist);
      }
    }
  }
}

SCENARIO("Relative include paths resolve from including file directory")
{
  GIVEN("A relative include path")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/b/CMakePresets.json"] =
      R"({"version":10,"include":["c/extra.json"]})";
    loader.Files["a/b/c/extra.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/b/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The include resolves relative to the parent file")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).FilePath
          == Abs("a/b/c/extra.json"));
      }
    }
  }
}

SCENARIO("fileDir macro is injected for include expansion")
{
  GIVEN("An include path that uses fileDir")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/b/CMakePresets.json"] =
      R"({"version":10,"include":["${fileDir}/extra.json"]})";
    loader.Files["a/b/extra.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/b/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The include resolves using fileDir")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).FilePath
          == Abs("a/b/extra.json"));
      }
    }
  }
}

SCENARIO("dollar macro is injected for include expansion")
{
  GIVEN("An include path that uses dollar")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["${dollar}special.json"]})";
    loader.Files["a/$special.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The include expands to a literal dollar")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).FilePath
          == Abs("a/$special.json"));
      }
    }
  }
}

SCENARIO("Manager reports $env include macros as unsupported")
{
  GIVEN("A root file with a $env include macro")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["$env{HOME}/extra.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The root file is unresolved with UnsupportedMacro")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}

SCENARIO("Manager reports preset-specific include macros as unsupported")
{
  GIVEN("A root file with a preset-specific include macro")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":10,"include":["${presetName}/extra.json"]})";

    auto context = MacroContext{};
    context.SetMacro("presetName", "debug");
    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(context);

      THEN("The root file is unresolved with UnsupportedMacro")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}

SCENARIO("Manager reports version 7 fileDir include macros as unsupported")
{
  GIVEN("A version 7 root file with a fileDir include macro")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakePresets.json"] =
      R"({"version":7,"include":["${fileDir}/extra.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("a/CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The root file is unresolved with UnsupportedMacro")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::UnsupportedMacro);
      }
    }
  }
}

SCENARIO("cmakeMinimumRequired is enforced")
{
  GIVEN("A file requiring newer CMake than simulated")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] =
      R"({"version":10,"cmakeMinimumRequired":{"major":3,"minor":30,"patch":0}})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 23});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is marked unresolved with CMakeMinimumRequiredNotMet")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::CMakeMinimumRequiredNotMet);
      }
    }
  }
}

SCENARIO("Unsupported preset version is reported")
{
  GIVEN("A file version not supported by simulated CMake")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({"version":9})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 23});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is marked unresolved with PresetVersionUnsupported")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::PresetVersionUnsupported);
      }
    }
  }
}

SCENARIO("CMake 4.2 supports preset file version 10")
{
  GIVEN("A manager simulating CMake 4.2")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({"version":11})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 4, .Minor = 2});
    manager.AddRootFile("CMakePresets.json");

    WHEN("A preset file version 11 file is loaded")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is rejected as newer than supported")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::PresetVersionUnsupported);
      }
    }
  }
}

SCENARIO("CMake 4.3 supports preset file version 11")
{
  GIVEN("A manager simulating CMake 4.3")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({"version":11})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 4, .Minor = 3});
    manager.AddRootFile("CMakePresets.json");

    WHEN("A preset file version 11 file is loaded")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is not rejected as newer than supported")
      {
        REQUIRE_FALSE(manager.GetIncludeGraph().GetFilePayload(0).IsUnresolved);
        REQUIRE_FALSE(
          manager.GetIncludeGraph().GetFilePayload(0).Reason.has_value());
      }
    }
  }
}

SCENARIO("Missing root version prevents include processing")
{
  GIVEN("A file without root version")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({"include":["extra.json"]})";
    loader.Files["extra.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is unresolved and no include is processed")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::PresetVersionMissing);
        REQUIRE(manager.GetIncludeGraph().FileCount() == 1U);
      }
    }
  }
}

SCENARIO("Include field is rejected for version below 4")
{
  GIVEN("A version 3 file with include field")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] =
      R"({"version":3,"include":["extra.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The file is unresolved with IncludeFieldUnsupportedInPresetVersion")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::IncludeFieldUnsupportedInPresetVersion);
      }
    }
  }
}

SCENARIO("Include cycles are detected")
{
  GIVEN("Two files including each other")
  {
    auto loader = TestFileLoader{};
    loader.Files["A.json"] = R"({"version":10,"include":["B.json"]})";
    loader.Files["B.json"] = R"({"version":10,"include":["A.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("A.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("Cycle reason is reported and manager is unresolved")
      {
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(0).Reason
          == UnresolvedReason::IncludeCycle);
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(1).Reason
          == UnresolvedReason::IncludeCycle);
        REQUIRE(manager.ComputeState() == PresetsGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("Applying context tolerates repeated inclusion")
{
  GIVEN("A file included directly and through another include")
  {
    auto loader = TestFileLoader{};
    loader.Files["A.json"] = R"({"version":10,"include":["B.json","C.json"]})";
    loader.Files["B.json"] = R"({"version":10})";
    loader.Files["C.json"] = R"({"version":10,"include":["B.json"]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("A.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The repeated file is loaded once and not reported as a cycle")
      {
        const auto repeatedFile =
          manager.GetIncludeGraph().FindFileNode(Abs("B.json"));
        REQUIRE(repeatedFile.has_value());
        REQUIRE(loader.LoadCountsByPath[Abs("B.json")] == 1U);
        REQUIRE(manager.GetIncludeGraph().GetFilePayload(*repeatedFile).Reason
          != UnresolvedReason::IncludeCycle);
      }
    }
  }
}

SCENARIO("Composite state is unresolved when inheritance graph is unresolved")
{
  GIVEN("A resolved include graph and unresolved inheritance graph")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] =
      R"({"version":10,"configurePresets":[{"name":"x","condition":{"type":"equals"}}]})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The manager composite state is unresolved")
      {
        REQUIRE(manager.GetIncludeGraph().ComputeState()
          == nhc::preset_graph::IncludeGraphState::Resolved);
        REQUIRE(manager.GetInheritanceGraph().ComputeState()
          == nhc::preset_graph::InheritanceGraphState::Unresolved);
        REQUIRE(manager.ComputeState() == PresetsGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("Root CMakeUserPresets implicitly includes sibling CMakePresets")
{
  GIVEN("A user presets root with a readable sibling project presets file")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakeUserPresets.json"] = R"({"version":10})";
    loader.Files["a/CMakePresets.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    const auto userId = manager.AddRootFile("a/CMakeUserPresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The sibling project presets file is included")
      {
        REQUIRE(manager.GetIncludeGraph().FileCount() == 2U);
        REQUIRE(manager.GetIncludeGraph().GetIncludedFiles(userId).size()
          == 1U);
        REQUIRE(
          manager.GetIncludeGraph()
            .GetFilePayload(
              manager.GetIncludeGraph().GetIncludedFiles(userId).front())
            .FilePath
          == Abs("a/CMakePresets.json"));
      }
    }
  }
}

SCENARIO("Root CMakeUserPresets does not include absent sibling")
{
  GIVEN("A user presets root without a readable sibling project presets file")
  {
    auto loader = TestFileLoader{};
    loader.Files["a/CMakeUserPresets.json"] = R"({"version":10})";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    const auto userId = manager.AddRootFile("a/CMakeUserPresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("No implicit include edge is created")
      {
        REQUIRE(manager.GetIncludeGraph().FileCount() == 1U);
        REQUIRE(manager.GetIncludeGraph().GetIncludedFiles(userId).empty());
      }
    }
  }
}

SCENARIO("Manager ingests all supported preset collections")
{
  GIVEN("A preset file containing one preset of each supported kind")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [{"name":"cfg","generator":"Ninja"}],
      "buildPresets": [{"name":"bld","configurePreset":"cfg"}],
      "testPresets": [{"name":"tst","configurePreset":"cfg"}],
      "packagePresets": [{"name":"pkg","configurePreset":"cfg"}],
      "workflowPresets": [{"name":"wrk","steps":[{"type":"configure","name":"cfg"}]}]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The model stores typed presets and workflow stays model-only")
      {
        REQUIRE(manager.GetPresetModel().GetPresetKind("cfg")
          == PresetKind::Configure);
        REQUIRE(manager.GetPresetModel().GetPresetKind("bld")
          == PresetKind::Build);
        REQUIRE(manager.GetPresetModel().GetPresetKind("tst")
          == PresetKind::Test);
        REQUIRE(manager.GetPresetModel().GetPresetKind("pkg")
          == PresetKind::Package);
        REQUIRE(manager.GetPresetModel().GetPresetKind("wrk")
          == PresetKind::Workflow);
        REQUIRE(manager.GetInheritanceGraph().ComputeState()
          == nhc::preset_graph::InheritanceGraphState::Resolved);
      }
    }
  }
}

SCENARIO("Manager parses preset conditions during ingestion")
{
  GIVEN("Presets with boolean, object, null, and invalid conditions")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [
        {"name":"enabled","condition":true},
        {"name":"matched","condition":{"type":"equals","lhs":"${presetName}","rhs":"matched"}},
        {"name":"cleared","condition":null},
        {"name":"bad","condition":{"type":"unknown"}}
      ]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      auto context = MacroContext{};
      context.SetMacro("presetName", "matched");
      manager.ApplyContext(context);

      THEN("Condition declarations and invalid diagnostics are retained")
      {
        REQUIRE(
          manager.GetPresetModel()
            .GetPreset<ConfigurePreset>("enabled")
            ->GetCondition()
          != nullptr);
        REQUIRE(
          manager.GetPresetModel()
            .GetPreset<ConfigurePreset>("cleared")
            ->GetConditionState()
          == nhc::preset_graph::PresetConditionState::ExplicitNull);
        REQUIRE(manager.GetPresetModel().GetPreset("bad").GetReason()
          == UnresolvedReason::InvalidCondition);
        REQUIRE(manager.ComputeState() == PresetsGraphState::Unresolved);
      }
    }
  }
}

SCENARIO("Manager publishes inherited false conditions for availability")
{
  GIVEN("A child preset inheriting a false condition")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [
        {"name":"P0","condition":false},
        {"name":"C","inherits":["P0"]}
      ]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The child preset is disabled by the inherited condition")
      {
        const auto& child = FindPayload<2>(manager.GetInheritanceGraph(), "C");
        REQUIRE(child.Availability == PresetAvailability::Disabled);
      }
    }
  }
}

SCENARIO("Manager leaves explicit null effective conditions active")
{
  GIVEN("A child preset clearing an inherited false condition")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [
        {"name":"P0","condition":false},
        {"name":"C","inherits":["P0"],"condition":null}
      ]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The cleared condition is absent for availability")
      {
        const auto& child = FindPayload<2>(manager.GetInheritanceGraph(), "C");
        REQUIRE(child.ConditionAst == nullptr);
        REQUIRE(child.Availability == PresetAvailability::Active);
      }
    }
  }
}

SCENARIO("Manager preserves inheritance cycle diagnostics after conditions")
{
  GIVEN("Presets with a direct inherits cycle")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [
        {"name":"A","inherits":["B"]},
        {"name":"B","inherits":["A"]}
      ]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("Both presets keep the InheritanceCycle diagnostic")
      {
        const auto& first = FindPayload<2>(manager.GetInheritanceGraph(), "A");
        const auto& second = FindPayload<2>(manager.GetInheritanceGraph(), "B");
        REQUIRE(first.IsUnresolved);
        REQUIRE(second.IsUnresolved);
        REQUIRE(first.Reason == UnresolvedReason::InheritanceCycle);
        REQUIRE(second.Reason == UnresolvedReason::InheritanceCycle);
      }
    }
  }
}

SCENARIO("Workflow validation records mismatched configure preset diagnostics")
{
  GIVEN("A workflow whose build step uses a different configure preset")
  {
    auto loader = TestFileLoader{};
    loader.Files["CMakePresets.json"] = R"({
      "version": 10,
      "configurePresets": [{"name":"cfg"},{"name":"other"}],
      "buildPresets": [{"name":"bld","configurePreset":"other"}],
      "workflowPresets": [{"name":"wrk","steps":[
        {"type":"configure","name":"cfg"},
        {"type":"build","name":"bld"}
      ]}]
    })";

    auto manager = PresetsGraph(loader, CMakeVersion{.Major = 3, .Minor = 31});
    manager.AddRootFile("CMakePresets.json");

    WHEN("Context is applied")
    {
      manager.ApplyContext(MacroContext{});

      THEN("The workflow remains queryable and receives a diagnostic")
      {
        REQUIRE(manager.GetPresetModel().GetPreset<WorkflowPreset>("wrk")
          != nullptr);
        REQUIRE_FALSE(manager.GetWorkflowDiagnostics().empty());
      }
    }
  }
}
