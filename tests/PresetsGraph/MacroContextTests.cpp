/// @file MacroContextTests.cpp
/// @brief Tests for MacroContext expansion behavior.

#include <catch2/catch_test_macros.hpp>

#include "MacroContext.h"

namespace {

using nhc::preset_graph::ExpansionStatus;
using nhc::preset_graph::MacroContext;

}  // namespace

SCENARIO("Macro context stores and retrieves macros")
{
  GIVEN("A macro context with a presetName macro")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    THEN("The stored macro can be queried")
    {
      auto value = context.GetMacro("presetName");
      REQUIRE(value.has_value());
      REQUIRE(value.value() == "default");
    }
  }
}

SCENARIO("Macro expansion resolves known macros")
{
  GIVEN("A macro context with a known macro")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    WHEN("An input string is expanded")
    {
      auto result = context.ExpandString("build-${presetName}");

      THEN("The string is fully expanded")
      {
        REQUIRE(result.ExpandedString == "build-default");
        REQUIRE(result.Status == ExpansionStatus::FullyExpanded);
        REQUIRE(result.UnresolvedTokens.empty());
      }
    }
  }
}

SCENARIO("Macro expansion reports unresolved tokens")
{
  GIVEN("A macro context missing a macro")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    WHEN("An input string references an unknown macro")
    {
      auto result = context.ExpandString("x-${unknownMacro}-y");

      THEN("The string is partially expanded and reports the token")
      {
        REQUIRE(result.ExpandedString == "x-${unknownMacro}-y");
        REQUIRE(result.Status == ExpansionStatus::PartiallyExpanded);
        REQUIRE(result.UnresolvedTokens
          == std::vector<std::string>{"${unknownMacro}"});
      }
    }
  }
}

SCENARIO("$env prefers preset environment values")
{
  GIVEN("Preset and parent environment values for the same key")
  {
    MacroContext context;
    context.SetPresetEnvironmentValue("VCPKG_ROOT", "./vcpkg-root");
    context.SetParentEnvironmentValue("VCPKG_ROOT", "C:/vcpkg");

    WHEN("An $env macro is expanded")
    {
      auto result = context.ExpandString("$env{VCPKG_ROOT}/scripts");

      THEN("The preset environment value is used")
      {
        REQUIRE(result.ExpandedString == "./vcpkg-root/scripts");
        REQUIRE(result.Status == ExpansionStatus::FullyExpanded);
        REQUIRE(result.UnresolvedTokens.empty());
      }
    }
  }
}

SCENARIO("$penv uses only parent environment values")
{
  GIVEN("Preset and parent environment values for the same key")
  {
    MacroContext context;
    context.SetPresetEnvironmentValue("VCPKG_ROOT", "./vcpkg-root");
    context.SetParentEnvironmentValue("VCPKG_ROOT", "C:/vcpkg");

    WHEN("A $penv macro is expanded")
    {
      auto result = context.ExpandString("$penv{VCPKG_ROOT}/scripts");

      THEN("The parent environment value is used")
      {
        REQUIRE(result.ExpandedString == "C:/vcpkg/scripts");
        REQUIRE(result.Status == ExpansionStatus::FullyExpanded);
        REQUIRE(result.UnresolvedTokens.empty());
      }
    }
  }
}

SCENARIO("Missing $penv variables remain unresolved")
{
  GIVEN("A context without a parent environment value")
  {
    MacroContext context;
    context.SetPresetEnvironmentValue("VCPKG_ROOT", "./vcpkg-root");

    WHEN("A $penv macro references a missing variable")
    {
      auto result = context.ExpandString("$penv{VCPKG_ROOT}/scripts");

      THEN("The token remains unresolved")
      {
        REQUIRE(result.ExpandedString == "$penv{VCPKG_ROOT}/scripts");
        REQUIRE(result.Status == ExpansionStatus::PartiallyExpanded);
        REQUIRE(result.UnresolvedTokens
          == std::vector<std::string>{"$penv{VCPKG_ROOT}"});
      }
    }
  }
}

SCENARIO("Missing $env variables remain unresolved")
{
  GIVEN("A context without preset or parent environment values")
  {
    MacroContext context;

    WHEN("An $env macro references a missing variable")
    {
      auto result = context.ExpandString("prefix-$env{DOES_NOT_EXIST}-suffix");

      THEN("The token remains unresolved")
      {
        REQUIRE(result.ExpandedString == "prefix-$env{DOES_NOT_EXIST}-suffix");
        REQUIRE(result.Status == ExpansionStatus::PartiallyExpanded);
        REQUIRE(result.UnresolvedTokens
          == std::vector<std::string>{"$env{DOES_NOT_EXIST}"});
      }
    }
  }
}
