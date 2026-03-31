/// @file Condition.cpp
/// @brief Implements condition AST node evaluation.

#include "Condition.h"

#include <algorithm>
#include <regex>
#include <utility>

namespace nhc::preset_graph {
namespace {

bool
IsUnknown(const ExpansionResult& result)
{
  return result.Status == ExpansionStatus::PartiallyExpanded;
}

ConditionResult
EvaluateInList(const MacroContext& context, const std::string& value,
  const std::vector<std::string>& values, bool invert)
{
  const auto expandedValue = context.ExpandString(value);
  if(IsUnknown(expandedValue)) {
    return ConditionResult::Unknown;
  }

  std::vector<std::string> expandedValues;
  expandedValues.reserve(values.size());
  for(const auto& candidate : values) {
    const auto expandedCandidate = context.ExpandString(candidate);
    if(IsUnknown(expandedCandidate)) {
      return ConditionResult::Unknown;
    }
    expandedValues.push_back(expandedCandidate.ExpandedString);
  }

  const bool isInList =
    std::find(expandedValues.begin(), expandedValues.end(),
      expandedValue.ExpandedString)
    != expandedValues.end();
  if(invert) {
    return isInList ? ConditionResult::False : ConditionResult::True;
  }

  return isInList ? ConditionResult::True : ConditionResult::False;
}

ConditionResult
EvaluateRegex(const MacroContext& context, const std::string& value,
  const std::string& pattern, bool invert)
{
  const auto expandedValue = context.ExpandString(value);
  const auto expandedPattern = context.ExpandString(pattern);
  if(IsUnknown(expandedValue) || IsUnknown(expandedPattern)) {
    return ConditionResult::Unknown;
  }

  const bool matches = std::regex_match(expandedValue.ExpandedString,
    std::regex(expandedPattern.ExpandedString));
  if(invert) {
    return matches ? ConditionResult::False : ConditionResult::True;
  }

  return matches ? ConditionResult::True : ConditionResult::False;
}

}  // namespace

ConstCondition::ConstCondition(bool value)
: m_Value(value)
{}

ConditionResult
ConstCondition::Evaluate(const MacroContext& /*context*/) const
{
  return m_Value ? ConditionResult::True : ConditionResult::False;
}

EqualsCondition::EqualsCondition(std::string left, std::string right)
: m_Left(std::move(left))
, m_Right(std::move(right))
{}

ConditionResult
EqualsCondition::Evaluate(const MacroContext& context) const
{
  const auto expandedLeft = context.ExpandString(m_Left);
  const auto expandedRight = context.ExpandString(m_Right);
  if(IsUnknown(expandedLeft) || IsUnknown(expandedRight)) {
    return ConditionResult::Unknown;
  }

  return expandedLeft.ExpandedString == expandedRight.ExpandedString
    ? ConditionResult::True
    : ConditionResult::False;
}

NotEqualsCondition::NotEqualsCondition(std::string left, std::string right)
: m_Left(std::move(left))
, m_Right(std::move(right))
{}

ConditionResult
NotEqualsCondition::Evaluate(const MacroContext& context) const
{
  const auto expandedLeft = context.ExpandString(m_Left);
  const auto expandedRight = context.ExpandString(m_Right);
  if(IsUnknown(expandedLeft) || IsUnknown(expandedRight)) {
    return ConditionResult::Unknown;
  }

  return expandedLeft.ExpandedString != expandedRight.ExpandedString
    ? ConditionResult::True
    : ConditionResult::False;
}

InListCondition::InListCondition(std::string value,
  std::vector<std::string> values)
: m_Value(std::move(value))
, m_Values(std::move(values))
{}

ConditionResult
InListCondition::Evaluate(const MacroContext& context) const
{
  return EvaluateInList(context, m_Value, m_Values, false);
}

NotInListCondition::NotInListCondition(std::string value,
  std::vector<std::string> values)
: m_Value(std::move(value))
, m_Values(std::move(values))
{}

ConditionResult
NotInListCondition::Evaluate(const MacroContext& context) const
{
  return EvaluateInList(context, m_Value, m_Values, true);
}

MatchesCondition::MatchesCondition(std::string value, std::string pattern)
: m_Value(std::move(value))
, m_Pattern(std::move(pattern))
{}

ConditionResult
MatchesCondition::Evaluate(const MacroContext& context) const
{
  return EvaluateRegex(context, m_Value, m_Pattern, false);
}

NotMatchesCondition::NotMatchesCondition(std::string value, std::string pattern)
: m_Value(std::move(value))
, m_Pattern(std::move(pattern))
{}

ConditionResult
NotMatchesCondition::Evaluate(const MacroContext& context) const
{
  return EvaluateRegex(context, m_Value, m_Pattern, true);
}

void
AllOfCondition::AddCondition(std::unique_ptr<Condition> condition)
{
  m_Children.push_back(std::move(condition));
}

ConditionResult
AllOfCondition::Evaluate(const MacroContext& context) const
{
  bool hasUnknown = false;
  for(const auto& child : m_Children) {
    const auto result = child->Evaluate(context);
    if(result == ConditionResult::False) {
      return ConditionResult::False;
    }
    if(result == ConditionResult::Unknown) {
      hasUnknown = true;
    }
  }

  return hasUnknown ? ConditionResult::Unknown : ConditionResult::True;
}

void
AnyOfCondition::AddCondition(std::unique_ptr<Condition> condition)
{
  m_Children.push_back(std::move(condition));
}

ConditionResult
AnyOfCondition::Evaluate(const MacroContext& context) const
{
  bool hasUnknown = false;
  for(const auto& child : m_Children) {
    const auto result = child->Evaluate(context);
    if(result == ConditionResult::True) {
      return ConditionResult::True;
    }
    if(result == ConditionResult::Unknown) {
      hasUnknown = true;
    }
  }

  return hasUnknown ? ConditionResult::Unknown : ConditionResult::False;
}

NotCondition::NotCondition(std::unique_ptr<Condition> child)
: m_Child(std::move(child))
{}

ConditionResult
NotCondition::Evaluate(const MacroContext& context) const
{
  const auto result = m_Child->Evaluate(context);
  if(result == ConditionResult::True) {
    return ConditionResult::False;
  }
  if(result == ConditionResult::False) {
    return ConditionResult::True;
  }
  return ConditionResult::Unknown;
}

}  // namespace nhc::preset_graph
