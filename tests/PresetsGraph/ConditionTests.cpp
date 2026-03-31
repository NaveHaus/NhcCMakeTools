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
using nhc::preset_graph::NotCondition;
using nhc::preset_graph::NotEqualsCondition;
using nhc::preset_graph::NotInListCondition;
using nhc::preset_graph::NotMatchesCondition;
using nhc::preset_graph::AnyOfCondition;

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

SCENARIO("NotEqualsCondition evaluates true for different values")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    WHEN("NotEqualsCondition compares different values")
    {
      const NotEqualsCondition condition("${presetName}", "other");

      THEN("The result is true")
      {
        REQUIRE(condition.Evaluate(context) == ConditionResult::True);
      }
    }
  }
}

SCENARIO("NotEqualsCondition evaluates false for equal values")
{
  GIVEN("A context containing presetName")
  {
    MacroContext context;
    context.SetMacro("presetName", "default");

    WHEN("NotEqualsCondition compares equal values")
    {
      const NotEqualsCondition condition("${presetName}", "default");

      THEN("The result is false")
      {
        REQUIRE(condition.Evaluate(context) == ConditionResult::False);
      }
    }
  }
}

SCENARIO("AnyOfCondition short-circuits to true")
{
  GIVEN("A logical any-of with a true condition")
  {
    MacroContext context;
    auto condition = AnyOfCondition{};
    condition.AddCondition(std::make_unique<ConstCondition>(true));
    condition.AddCondition(std::make_unique<EqualsCondition>("${unknownMacro}",
      "x"));

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

SCENARIO("AnyOfCondition evaluates false when all children are false")
{
  GIVEN("A logical any-of with all false conditions")
  {
    MacroContext context;
    auto condition = AnyOfCondition{};
    condition.AddCondition(std::make_unique<ConstCondition>(false));
    condition.AddCondition(std::make_unique<ConstCondition>(false));

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

SCENARIO("AnyOfCondition evaluates unknown when no true but has unknown")
{
  GIVEN("A logical any-of with false and unknown conditions")
  {
    MacroContext context;
    auto condition = AnyOfCondition{};
    condition.AddCondition(std::make_unique<ConstCondition>(false));
    condition.AddCondition(std::make_unique<EqualsCondition>("${unknownMacro}",
      "x"));

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is unknown")
      {
        REQUIRE(result == ConditionResult::Unknown);
      }
    }
  }
}

SCENARIO("NotCondition inverts true to false")
{
  GIVEN("A not condition wrapping true")
  {
    MacroContext context;
    auto condition = NotCondition{std::make_unique<ConstCondition>(true)};

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

SCENARIO("NotCondition inverts false to true")
{
  GIVEN("A not condition wrapping false")
  {
    MacroContext context;
    auto condition = NotCondition{std::make_unique<ConstCondition>(false)};

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

SCENARIO("NotCondition preserves unknown")
{
  GIVEN("A not condition wrapping unknown")
  {
    MacroContext context;
    auto condition =
      NotCondition{std::make_unique<EqualsCondition>("${unknownMacro}", "x")};

    WHEN("The condition is evaluated")
    {
      const auto result = condition.Evaluate(context);

      THEN("The result is unknown")
      {
        REQUIRE(result == ConditionResult::Unknown);
      }
    }
  }
}
