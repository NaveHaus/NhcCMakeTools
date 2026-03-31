/// @file GraphManagerTests.cpp
/// @brief Tests for PresetsGraph manager behavior.

#include <catch2/catch_test_macros.hpp>

#include <unordered_map>

#include "PresetsGraph.h"

namespace {

using nhc::preset_graph::CMakeVersion;
using nhc::preset_graph::FileLoadResult;
using nhc::preset_graph::FileLoader;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::PresetsGraph;
using nhc::preset_graph::PresetsGraphState;
using nhc::preset_graph::UnresolvedReason;

class TestFileLoader final : public FileLoader
{
  public:
  mutable unsigned LoadCount = 0;
  std::unordered_map<std::string, std::string> Files;

  FileLoadResult LoadFile(const std::string& path) const override
  {
    ++LoadCount;
    const auto it = Files.find(path);
    if(it == Files.end()) {
      return FileLoadResult{.Success = false, .FileDoesNotExist = true};
    }

    return FileLoadResult{.Success = true, .Contents = it->second};
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
          == "a/b/c/extra.json");
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
          == "a/b/extra.json");
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
          == "a/$special.json");
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
