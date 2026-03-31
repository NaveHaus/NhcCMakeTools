/// @file PresetModelTests.cpp
/// @brief Tests for preset model resolution behavior.

#include <catch2/catch_test_macros.hpp>

#include "PresetModel.h"

namespace {

using nhc::preset_graph::MacroContext;
using nhc::preset_graph::Preset;
using nhc::preset_graph::PresetModel;
using nhc::preset_graph::PresetType;
using nhc::preset_graph::UnresolvedReason;

}  // namespace

SCENARIO("Preset type is preserved from source array kind")
{
  GIVEN("A preset model containing all preset kinds")
  {
    PresetModel model;
    model.AddPreset(Preset{.Name = "cfg", .Type = PresetType::Configure});
    model.AddPreset(Preset{.Name = "bld", .Type = PresetType::Build});
    model.AddPreset(Preset{.Name = "tst", .Type = PresetType::Test});
    model.AddPreset(Preset{.Name = "pkg", .Type = PresetType::Package});
    model.AddPreset(Preset{.Name = "wrk", .Type = PresetType::Workflow});

    THEN("Each preset reports its declared type")
    {
      REQUIRE(model.GetPresetType("cfg") == PresetType::Configure);
      REQUIRE(model.GetPresetType("bld") == PresetType::Build);
      REQUIRE(model.GetPresetType("tst") == PresetType::Test);
      REQUIRE(model.GetPresetType("pkg") == PresetType::Package);
      REQUIRE(model.GetPresetType("wrk") == PresetType::Workflow);
    }
  }
}

SCENARIO("Earlier inherits entry wins scalar conflicts")
{
  GIVEN("A child inheriting from two parents with conflicting installDir")
  {
    PresetModel model;
    model.AddPreset(Preset{.Name = "P0", .InstallDir = "p0"});
    model.AddPreset(Preset{.Name = "P1", .InstallDir = "p1"});
    model.AddPreset(Preset{
      .Name = "C", .Type = PresetType::Configure, .Inherits = {"P0", "P1"}});

    WHEN("The child is resolved")
    {
      const auto resolved = model.ResolvePreset("C");

      THEN("The first parent installDir is used")
      {
        REQUIRE(resolved.InstallDir == std::optional<std::string>{"p0"});
      }
    }
  }
}

SCENARIO("Environment merge honors parent precedence and null removal")
{
  GIVEN("A child with inherited environment and null override")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "P0",
      .Environment = {{"X", std::optional<std::string>{"0"}}},
    });
    model.AddPreset(Preset{
      .Name = "P1",
      .Environment = {{"X", std::optional<std::string>{"1"}}},
    });
    model.AddPreset(Preset{
      .Name = "C",
      .Inherits = {"P0", "P1"},
      .Environment = {{"X", std::nullopt}},
    });

    WHEN("The child is resolved")
    {
      const auto resolved = model.ResolvePreset("C");

      THEN("The key is removed by null override")
      {
        REQUIRE_FALSE(resolved.Environment.contains("X"));
      }
    }
  }
}

SCENARIO("Build preset merges configure environment in middle")
{
  GIVEN("A build preset with inherited, configure, and explicit environment")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Type = PresetType::Configure,
      .Environment = {{"X", std::optional<std::string>{"cfg"}}},
    });
    model.AddPreset(Preset{
      .Name = "P0",
      .Type = PresetType::Build,
      .Environment =
        {
          {"X", std::optional<std::string>{"p0"}},
          {"Y", std::optional<std::string>{"p0"}},
        },
    });
    model.AddPreset(Preset{
      .Name = "bld",
      .Type = PresetType::Build,
      .Inherits = {"P0"},
      .ConfigurePreset = "cfg",
      .InheritConfigureEnvironment = true,
      .Environment = {{"Y", std::optional<std::string>{"bld"}}},
    });

    WHEN("The build preset is resolved")
    {
      const auto resolved = model.ResolvePreset("bld");

      THEN(
        "Configure values override inherited values before explicit overrides")
      {
        REQUIRE(resolved.Environment.at("X") == "cfg");
        REQUIRE(resolved.Environment.at("Y") == "bld");
      }
    }
  }
}

SCENARIO("Environment supports out-of-order $env references")
{
  GIVEN("Environment values referencing each other")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Environment =
        {
          {"A", std::optional<std::string>{"$env{B}"}},
          {"B", std::optional<std::string>{"b"}},
        },
    });

    WHEN("The preset is resolved")
    {
      const auto resolved = model.ResolvePreset("cfg");

      THEN("Dependent values are expanded")
      {
        REQUIRE(resolved.Environment.at("A") == "b");
      }
    }
  }
}

SCENARIO("Environment reference cycles are detected")
{
  GIVEN("Environment values with a cycle")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Environment =
        {
          {"A", std::optional<std::string>{"$env{B}"}},
          {"B", std::optional<std::string>{"$env{A}"}},
        },
    });

    WHEN("The preset is resolved")
    {
      const auto resolved = model.ResolvePreset("cfg");

      THEN("The preset reports environment cycle reason")
      {
        REQUIRE(resolved.Reason == UnresolvedReason::EnvironmentCycle);
      }
    }
  }
}

SCENARIO("Inherited values expand with active presetName")
{
  GIVEN("A child inheriting presetName-dependent environment")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "P0",
      .Environment = {{"X", std::optional<std::string>{"${presetName}"}}},
    });
    model.AddPreset(Preset{
      .Name = "child",
      .Inherits = {"P0"},
    });

    WHEN("The child preset is resolved")
    {
      const auto resolved = model.ResolvePreset("child");

      THEN("The inherited value uses the child presetName")
      {
        REQUIRE(resolved.Environment.at("X") == "child");
      }
    }
  }
}

SCENARIO("Configure preset injects generator macro")
{
  GIVEN("A configure preset with a generator")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Type = PresetType::Configure,
      .Generator = "Ninja",
      .Environment = {{"G", std::optional<std::string>{"${generator}"}}},
    });

    WHEN("The preset is resolved")
    {
      const auto resolved = model.ResolvePreset("cfg");

      THEN("The generator macro expands")
      {
        REQUIRE(resolved.Environment.at("G") == "Ninja");
      }
    }
  }
}

SCENARIO("Build preset derives generator from configurePreset")
{
  GIVEN("A build preset associated with configure preset")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Type = PresetType::Configure,
      .Generator = "Ninja",
    });
    model.AddPreset(Preset{
      .Name = "bld",
      .Type = PresetType::Build,
      .ConfigurePreset = "cfg",
      .Environment = {{"G", std::optional<std::string>{"${generator}"}}},
    });

    WHEN("The build preset is resolved")
    {
      const auto resolved = model.ResolvePreset("bld");

      THEN("The generator macro comes from configure preset")
      {
        REQUIRE(resolved.Environment.at("G") == "Ninja");
      }
    }
  }
}

SCENARIO("Raw preset is accessible after adding")
{
  GIVEN("A preset model with a preset containing unexpanded macros")
  {
    PresetModel model;
    model.AddPreset(Preset{
      .Name = "cfg",
      .Type = PresetType::Configure,
      .Inherits = {"base"},
      .InstallDir = "${sourceDir}/install",
      .Generator = "Ninja",
      .ConfigurePreset = std::nullopt,
      .InheritConfigureEnvironment = false,
      .Environment = {{"PATH", std::optional<std::string>{"${sourceDir}/bin"}}},
    });

    WHEN("The raw preset is retrieved")
    {
      const auto& raw = model.GetPreset("cfg");

      THEN("All original values are preserved unexpanded")
      {
        REQUIRE(raw.Name == "cfg");
        REQUIRE(raw.Type == PresetType::Configure);
        REQUIRE(raw.Inherits == std::vector<std::string>{"base"});
        REQUIRE(raw.InstallDir
          == std::optional<std::string>{"${sourceDir}/install"});
        REQUIRE(raw.Generator == std::optional<std::string>{"Ninja"});
        REQUIRE(raw.Environment.at("PATH")
          == std::optional<std::string>{"${sourceDir}/bin"});
      }
    }
  }
}
