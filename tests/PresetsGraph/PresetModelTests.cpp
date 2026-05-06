/// @file PresetModelTests.cpp
/// @brief Tests for preset model resolution behavior.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <nlohmann/json.hpp>
#include <type_traits>

#include "PresetModel.h"

namespace {

using nhc::preset_graph::BuildPreset;
using nhc::preset_graph::ConfigurePreset;
using nhc::preset_graph::EqualsCondition;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::PackagePreset;
using nhc::preset_graph::Preset;
using nhc::preset_graph::PresetConditionState;
using nhc::preset_graph::PresetKind;
using nhc::preset_graph::PresetModel;
using nhc::preset_graph::ResolvedFieldStatus;
using nhc::preset_graph::TestPreset;
using nhc::preset_graph::UnresolvedReason;
using nhc::preset_graph::WorkflowPreset;
using nhc::preset_graph::WorkflowStep;
using nhc::preset_graph::WorkflowStepType;

template<typename TPreset>
std::unique_ptr<TPreset>
MakeNamedPreset(const std::string& name)
{
  auto preset = std::make_unique<TPreset>();
  preset->SetName(name);
  return preset;
}

template<typename TPreset>
concept HasGetHidden = requires(const TPreset& preset) { preset.GetHidden(); };

template<typename TPreset>
concept HasGetInherits = requires(const TPreset& preset) {
  preset.GetInherits();
};

template<typename TPreset>
concept HasGetCondition = requires(const TPreset& preset) {
  preset.GetCondition();
};

template<typename TPreset>
concept HasGetConditionState = requires(const TPreset& preset) {
  preset.GetConditionState();
};

template<typename TPreset>
concept HasSetConditionExplicitNull = requires(TPreset& preset) {
  preset.SetConditionExplicitNull();
};

template<typename TPreset>
concept HasClearCondition = requires(TPreset& preset) {
  preset.ClearCondition();
};

template<typename TPreset>
concept HasGetEnvironment = requires(const TPreset& preset) {
  preset.GetEnvironment();
};

}  // namespace

SCENARIO("Preset hierarchy exposes common and type-specific fields")
{
  GIVEN("Preset instances from each supported kind")
  {
    auto base = MakeNamedPreset<ConfigurePreset>("cfg");
    base->SetHidden(true);
    base->SetInherits({"base"});
    base->SetCondition(std::make_unique<EqualsCondition>("x", "x"));
    base->SetEnvironment({{"PATH", std::optional<std::string>{"/tmp/bin"}}});

    auto build = MakeNamedPreset<BuildPreset>("bld");
    build->SetConfigurePreset("cfg");
    build->SetInheritConfigureEnvironment(false);

    auto test = MakeNamedPreset<TestPreset>("tst");
    test->SetConfigurePreset("cfg");

    auto package = MakeNamedPreset<PackagePreset>("pkg");
    package->SetConfigurePreset("cfg");

    auto workflow = MakeNamedPreset<WorkflowPreset>("wrk");
    workflow->SetSteps({WorkflowStep{
      .Type = WorkflowStepType::Configure,
      .Name = "cfg",
    }});

    THEN("Common fields and concrete kinds are available")
    {
      REQUIRE(base->GetName() == "cfg");
      REQUIRE(base->GetHidden());
      REQUIRE(base->GetInherits() == std::vector<std::string>{"base"});
      REQUIRE(base->GetCondition() != nullptr);
      REQUIRE(base->GetEnvironment().at("PATH")
        == std::optional<std::string>{"/tmp/bin"});
      REQUIRE(base->GetType() == PresetKind::Configure);
      REQUIRE(build->GetType() == PresetKind::Build);
      REQUIRE(test->GetType() == PresetKind::Test);
      REQUIRE(package->GetType() == PresetKind::Package);
      REQUIRE(workflow->GetType() == PresetKind::Workflow);
    }

    THEN("Derived presets expose their own typed accessors")
    {
      REQUIRE(build->GetConfigurePreset() == std::optional<std::string>{"cfg"});
      REQUIRE_FALSE(build->GetInheritConfigureEnvironment());
      REQUIRE(test->GetConfigurePreset() == std::optional<std::string>{"cfg"});
      REQUIRE(package->GetConfigurePreset()
        == std::optional<std::string>{"cfg"});
    }

    THEN("Workflow preset typed API exposes only name and steps")
    {
      static_assert(!HasGetHidden<WorkflowPreset>);
      static_assert(!HasGetInherits<WorkflowPreset>);
      static_assert(!HasGetCondition<WorkflowPreset>);
      static_assert(!HasGetConditionState<WorkflowPreset>);
      static_assert(!HasSetConditionExplicitNull<WorkflowPreset>);
      static_assert(!HasClearCondition<WorkflowPreset>);
      static_assert(!HasGetEnvironment<WorkflowPreset>);
      REQUIRE(workflow->GetName() == "wrk");
      REQUIRE(workflow->GetSteps().size() == 1);
      REQUIRE(workflow->GetSteps().front().Type == WorkflowStepType::Configure);
      REQUIRE(workflow->GetSteps().front().Name == "cfg");
    }
  }
}

SCENARIO("Configure preset exposes generator and installDir")
{
  GIVEN("A configure preset with configure-specific fields")
  {
    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetGenerator("Ninja");
    preset->SetInstallDir("install");

    THEN("The fields are available through the typed API")
    {
      REQUIRE(preset->GetGenerator() == std::optional<std::string>{"Ninja"});
      REQUIRE(preset->GetInstallDir() == std::optional<std::string>{"install"});
    }
  }
}

SCENARIO("Preset model stores presets polymorphically")
{
  GIVEN("A preset model receiving base-class unique_ptr values")
  {
    PresetModel model;

    std::unique_ptr<Preset> configure = MakeNamedPreset<ConfigurePreset>("cfg");
    model.AddPreset(std::move(configure));
    model.AddPreset(MakeNamedPreset<BuildPreset>("bld"));
    model.AddPreset(MakeNamedPreset<TestPreset>("tst"));
    model.AddPreset(MakeNamedPreset<PackagePreset>("pkg"));
    model.AddPreset(MakeNamedPreset<WorkflowPreset>("wrk"));

    THEN("Each preset reports its declared kind")
    {
      REQUIRE(model.GetPresetKind("cfg") == PresetKind::Configure);
      REQUIRE(model.GetPresetKind("bld") == PresetKind::Build);
      REQUIRE(model.GetPresetKind("tst") == PresetKind::Test);
      REQUIRE(model.GetPresetKind("pkg") == PresetKind::Package);
      REQUIRE(model.GetPresetKind("wrk") == PresetKind::Workflow);
    }

    THEN("Typed retrieval returns the correct derived type")
    {
      REQUIRE(model.GetPreset<ConfigurePreset>("cfg") != nullptr);
      REQUIRE(model.GetPreset<BuildPreset>("bld") != nullptr);
      REQUIRE(model.GetPreset<TestPreset>("tst") != nullptr);
      REQUIRE(model.GetPreset<PackagePreset>("pkg") != nullptr);
      REQUIRE(model.GetPreset<WorkflowPreset>("wrk") != nullptr);
    }

    THEN("Wrong typed retrieval returns null")
    {
      REQUIRE(model.GetPreset<BuildPreset>("cfg") == nullptr);
    }
  }
}

SCENARIO("Earlier inherits entry wins scalar conflicts")
{
  GIVEN("A child inheriting from two parents with conflicting installDir")
  {
    PresetModel model;

    auto parentZero = MakeNamedPreset<ConfigurePreset>("P0");
    parentZero->SetInstallDir("p0");
    model.AddPreset(std::move(parentZero));

    auto parentOne = MakeNamedPreset<ConfigurePreset>("P1");
    parentOne->SetInstallDir("p1");
    model.AddPreset(std::move(parentOne));

    auto child = MakeNamedPreset<ConfigurePreset>("C");
    child->SetInherits({"P0", "P1"});
    model.AddPreset(std::move(child));

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

    auto parentZero = MakeNamedPreset<ConfigurePreset>("P0");
    parentZero->SetEnvironment({{"X", std::optional<std::string>{"0"}}});
    model.AddPreset(std::move(parentZero));

    auto parentOne = MakeNamedPreset<ConfigurePreset>("P1");
    parentOne->SetEnvironment({{"X", std::optional<std::string>{"1"}}});
    model.AddPreset(std::move(parentOne));

    auto child = MakeNamedPreset<ConfigurePreset>("C");
    child->SetInherits({"P0", "P1"});
    child->SetEnvironment({{"X", std::nullopt}});
    model.AddPreset(std::move(child));

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

    auto configure = MakeNamedPreset<ConfigurePreset>("cfg");
    configure->SetEnvironment({{"X", std::optional<std::string>{"cfg"}}});
    model.AddPreset(std::move(configure));

    auto parent = MakeNamedPreset<BuildPreset>("P0");
    parent->SetEnvironment({
      {"X", std::optional<std::string>{"p0"}},
      {"Y", std::optional<std::string>{"p0"}},
    });
    model.AddPreset(std::move(parent));

    auto build = MakeNamedPreset<BuildPreset>("bld");
    build->SetInherits({"P0"});
    build->SetConfigurePreset("cfg");
    build->SetInheritConfigureEnvironment(true);
    build->SetEnvironment({{"Y", std::optional<std::string>{"bld"}}});
    model.AddPreset(std::move(build));

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

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetEnvironment({
      {"A", std::optional<std::string>{"$env{B}"}},
      {"B", std::optional<std::string>{"b"}},
    });
    model.AddPreset(std::move(preset));

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

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetEnvironment({
      {"A", std::optional<std::string>{"$env{B}"}},
      {"B", std::optional<std::string>{"$env{A}"}},
    });
    model.AddPreset(std::move(preset));

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

    auto parent = MakeNamedPreset<ConfigurePreset>("P0");
    parent->SetEnvironment({{"X",
      std::optional<std::string>{"${presetName}"}}});
    model.AddPreset(std::move(parent));

    auto child = MakeNamedPreset<ConfigurePreset>("child");
    child->SetInherits({"P0"});
    model.AddPreset(std::move(child));

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

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetGenerator("Ninja");
    preset->SetEnvironment({{"G", std::optional<std::string>{"${generator}"}}});
    model.AddPreset(std::move(preset));

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

    auto configure = MakeNamedPreset<ConfigurePreset>("cfg");
    configure->SetGenerator("Ninja");
    model.AddPreset(std::move(configure));

    auto build = MakeNamedPreset<BuildPreset>("bld");
    build->SetConfigurePreset("cfg");
    build->SetEnvironment({{"G", std::optional<std::string>{"${generator}"}}});
    model.AddPreset(std::move(build));

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

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetInherits({"base"});
    preset->SetInstallDir("${sourceDir}/install");
    preset->SetGenerator("Ninja");
    preset->SetEnvironment({{"PATH",
      std::optional<std::string>{"${sourceDir}/bin"}}});
    model.AddPreset(std::move(preset));

    WHEN("The raw preset is retrieved")
    {
      const auto& raw = model.GetPreset("cfg");
      const auto* configure = model.GetPreset<ConfigurePreset>("cfg");

      THEN("All original values are preserved unexpanded")
      {
        REQUIRE(raw.GetName() == "cfg");
        REQUIRE(raw.GetType() == PresetKind::Configure);
        REQUIRE(raw.GetInherits() == std::vector<std::string>{"base"});
        REQUIRE(configure != nullptr);
        REQUIRE(configure->GetInstallDir()
          == std::optional<std::string>{"${sourceDir}/install"});
        REQUIRE(configure->GetGenerator()
          == std::optional<std::string>{"Ninja"});
        REQUIRE(raw.GetEnvironment().at("PATH")
          == std::optional<std::string>{"${sourceDir}/bin"});
      }
    }
  }
}

SCENARIO(
  "Explicit null condition clears inherited condition for current preset")
{
  GIVEN("A child preset with condition null inheriting a false condition")
  {
    PresetModel model;

    auto parent = MakeNamedPreset<ConfigurePreset>("P0");
    parent->SetCondition(
      std::make_unique<nhc::preset_graph::ConstCondition>(false));
    model.AddPreset(std::move(parent));

    auto child = MakeNamedPreset<ConfigurePreset>("C");
    child->SetInherits({"P0"});
    child->SetConditionExplicitNull();
    model.AddPreset(std::move(child));

    WHEN("The child's effective condition is resolved")
    {
      const auto* condition = model.ResolveCondition("C");

      THEN("No inherited evaluable condition remains")
      {
        REQUIRE(condition == nullptr);
      }
    }
  }
}

SCENARIO("Effective condition resolution terminates on inherits cycles")
{
  GIVEN("Presets with a direct inherits cycle and no local conditions")
  {
    PresetModel model;

    auto first = MakeNamedPreset<ConfigurePreset>("A");
    first->SetInherits({"B"});
    model.AddPreset(std::move(first));

    auto second = MakeNamedPreset<ConfigurePreset>("B");
    second->SetInherits({"A"});
    model.AddPreset(std::move(second));

    WHEN("The effective condition is resolved")
    {
      const auto* condition = model.ResolveCondition("A");

      THEN("Lookup completes with no inherited condition")
      {
        REQUIRE(condition == nullptr);
      }
    }
  }
}

SCENARIO("Explicit null condition is not inherited by descendants")
{
  GIVEN("A grandchild inheriting through a parent with condition null")
  {
    PresetModel model;

    auto parent = MakeNamedPreset<ConfigurePreset>("P0");
    parent->SetCondition(
      std::make_unique<nhc::preset_graph::ConstCondition>(false));
    model.AddPreset(std::move(parent));

    auto child = MakeNamedPreset<ConfigurePreset>("C");
    child->SetInherits({"P0"});
    child->SetConditionExplicitNull();
    model.AddPreset(std::move(child));

    auto grandchild = MakeNamedPreset<ConfigurePreset>("G");
    grandchild->SetInherits({"C"});
    model.AddPreset(std::move(grandchild));

    WHEN("The grandchild's effective condition is resolved")
    {
      const auto* condition = model.ResolveCondition("G");

      THEN("It does not inherit an explicit null marker")
      {
        REQUIRE(condition == nullptr);
        REQUIRE(model.GetPreset("C").GetConditionState()
          == PresetConditionState::ExplicitNull);
      }
    }
  }
}

SCENARIO("Preset owns raw JSON and current resolved scalar state")
{
  GIVEN("A configure preset with a macro-bearing raw binaryDir")
  {
    PresetModel model;

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetRawJson(nlohmann::json{
      {"name", "cfg"},
      {"binaryDir", "${sourceDir}/build/${unknown}"},
    });
    model.AddPreset(std::move(preset));

    WHEN("The preset resolved state is refreshed")
    {
      auto context = MacroContext{};
      context.SetMacro("sourceDir", "/src");
      model.RefreshResolvedState("cfg", context);

      THEN("The preset retains raw JSON and partially resolved field state")
      {
        const auto& raw = model.GetPreset("cfg").GetRawJson();
        const auto& resolved = model.GetPreset("cfg").GetResolvedFields();

        REQUIRE(raw.at("binaryDir") == "${sourceDir}/build/${unknown}");
        REQUIRE(resolved.at("binaryDir").Value == "/src/build/${unknown}");
        REQUIRE(resolved.at("binaryDir").Status
          == ResolvedFieldStatus::PartiallyResolved);
      }
    }
  }
}

SCENARIO("Preset resolved state preserves structured JSON fields")
{
  GIVEN("A configure preset with structured cacheVariables")
  {
    PresetModel model;

    auto preset = MakeNamedPreset<ConfigurePreset>("cfg");
    preset->SetRawJson(nlohmann::json{
      {"name", "cfg"},
      {"cacheVariables", nlohmann::json{{"FEATURE", true}}},
    });
    model.AddPreset(std::move(preset));

    WHEN("The preset resolved state is refreshed")
    {
      model.RefreshResolvedState("cfg", MacroContext{});

      THEN("The structured field is retained as JSON")
      {
        const auto& resolved = model.GetPreset("cfg").GetResolvedFields();
        REQUIRE(model.GetPreset("cfg").GetRawJson().at("cacheVariables")
          == nlohmann::json{{"FEATURE", true}});
        REQUIRE(resolved.at("cacheVariables").Value
          == nlohmann::json{{"FEATURE", true}});
        REQUIRE(resolved.at("cacheVariables").Status
          == ResolvedFieldStatus::FullyResolved);
      }
    }
  }
}
