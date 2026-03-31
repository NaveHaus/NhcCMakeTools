/// @file ConditionTests.cpp
/// @brief Tests for condition AST evaluation.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "Condition.h"
#include "MacroContext.h"

namespace {

using nhc::preset_graph::AllOfCondition;
using nhc::preset_graph::ConditionResult;
using nhc::preset_graph::ConstCondition;
using nhc::preset_graph::EqualsCondition;
using nhc::preset_graph::InListCondition;
using nhc::preset_graph::MacroContext;
using nhc::preset_graph::MatchesCondition;
using nhc::preset_graph::NotInListCondition;
using nhc::preset_graph::NotMatchesCondition;

}  // namespace

SCENARIO("EqualsCondition evaluates true for equal expanded values")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    WHEN("EqualsCondition compares matching values")
    {
      const EqualsCondition condition("${presetName}", "default");

      THEN("The result is true")
      {
        REQUIRE(condition.Evaluate(context) == ConditionResult::True);
      }
    }
  }
}

SCENARIO("Condition evaluation returns unknown for missing macro")
{
  GIVEN("A context without unknownMacro")
  {
    MacroContext context;

    WHEN("EqualsCondition requires the missing macro")
    {
      const EqualsCondition condition("${unknownMacro}", "x");

      THEN("The result is unknown")
      {
        REQUIRE(condition.Evaluate(context) == ConditionResult::Unknown);
      }
    }
  }
}

SCENARIO("AllOfCondition short-circuits to false")
{
  GIVEN("A logical all-of with a false condition")
  {
    MacroContext context;
    auto condition = AllOfCondition{};
    condition.AddCondition(std::make_unique<ConstCondition>(false));
    condition.AddCondition(std::make_unique<EqualsCondition>("${unknownMacro}",
      "x"));

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is false")
      {
        REQUIRE(result == ConditionResult::False);
      }
    }
  }
}

SCENARIO("ConstCondition evaluates to its configured value")
{
  GIVEN("A constant true condition")
  {
    MacroContext context;
    const ConstCondition condition(true);

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is true")
      {
        REQUIRE(result == ConditionResult::True);
      }
    }
  }
}

SCENARIO("InListCondition evaluates true when value is present")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");
    const InListCondition condition("${presetName}",
      std::vector<std::string>{"default", "other"});

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is true")
      {
        REQUIRE(result == ConditionResult::True);
      }
    }
  }
}

SCENARIO("NotInListCondition evaluates true when value is absent")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");
    const NotInListCondition condition("${presetName}",
      std::vector<std::string>{"other"});

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is true")
      {
        REQUIRE(result == ConditionResult::True);
      }
    }
  }
}

SCENARIO("MatchesCondition evaluates true when regex matches")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");
    const MatchesCondition condition("${presetName}", "def.*");

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is true")
      {
        REQUIRE(result == ConditionResult::True);
      }
    }
  }
}

SCENARIO("NotMatchesCondition evaluates true when regex does not match")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");
    const NotMatchesCondition condition("${presetName}", "zzz.*");

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is true")
      {
        REQUIRE(result == ConditionResult::True);
      }
    }
  }
}
