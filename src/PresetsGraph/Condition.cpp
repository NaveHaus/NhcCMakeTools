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

bool
HasStringMembers(const nlohmann::json& value, const std::string& first,
  const std::string& second)
{
  return value.contains(first) && value[first].is_string()
    && value.contains(second) && value[second].is_string();
}

std::vector<std::string>
ReadStringList(const nlohmann::json& value)
{
  std::vector<std::string> result;
  if(!value.is_array()) {
    return result;
  }

  for(const auto& entry : value) {
    if(!entry.is_string()) {
      result.clear();
      return result;
    }
    result.push_back(entry.get<std::string>());
  }

  return result;
}

ConditionParseResult
ParseConditionJson(const nlohmann::json& value, bool allowExplicitNull)
{
  if(value.is_boolean()) {
    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst = std::make_unique<ConstCondition>(value.get<bool>()),
    };
  }

  if(value.is_null()) {
    return ConditionParseResult{
      .Status = allowExplicitNull
        ? ConditionParseStatus::ExplicitNull
        : ConditionParseStatus::Invalid,
      .ConditionAst = nullptr,
    };
  }

  if(!value.is_object() || !value.contains("type")
    || !value["type"].is_string())
  {
    return ConditionParseResult{};
  }

  const auto type = value["type"].get<std::string>();
  if(type == "const") {
    if(!value.contains("value") || !value["value"].is_boolean()) {
      return ConditionParseResult{};
    }
    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst =
        std::make_unique<ConstCondition>(value["value"].get<bool>()),
    };
  }

  if(type == "equals" || type == "notEquals") {
    if(!HasStringMembers(value, "lhs", "rhs")) {
      return ConditionParseResult{};
    }
    if(type == "equals") {
      return ConditionParseResult{
        .Status = ConditionParseStatus::Parsed,
        .ConditionAst = std::make_unique<EqualsCondition>(
          value["lhs"].get<std::string>(), value["rhs"].get<std::string>()),
      };
    }
    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst = std::make_unique<NotEqualsCondition>(
        value["lhs"].get<std::string>(), value["rhs"].get<std::string>()),
    };
  }

  if(type == "inList" || type == "notInList") {
    if(!value.contains("string") || !value["string"].is_string()
      || !value.contains("list") || !value["list"].is_array())
    {
      return ConditionParseResult{};
    }

    auto list = ReadStringList(value["list"]);
    if(list.size() != value["list"].size()) {
      return ConditionParseResult{};
    }

    if(type == "inList") {
      return ConditionParseResult{
        .Status = ConditionParseStatus::Parsed,
        .ConditionAst = std::make_unique<InListCondition>(
          value["string"].get<std::string>(), std::move(list)),
      };
    }
    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst = std::make_unique<NotInListCondition>(
        value["string"].get<std::string>(), std::move(list)),
    };
  }

  if(type == "matches" || type == "notMatches") {
    if(!HasStringMembers(value, "string", "regex")) {
      return ConditionParseResult{};
    }
    if(type == "matches") {
      return ConditionParseResult{
        .Status = ConditionParseStatus::Parsed,
        .ConditionAst = std::make_unique<MatchesCondition>(
          value["string"].get<std::string>(),
          value["regex"].get<std::string>()),
      };
    }
    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst = std::make_unique<NotMatchesCondition>(
        value["string"].get<std::string>(), value["regex"].get<std::string>()),
    };
  }

  if(type == "allOf" || type == "anyOf") {
    if(!value.contains("conditions") || !value["conditions"].is_array()) {
      return ConditionParseResult{};
    }

    std::unique_ptr<Condition> compound;
    if(type == "allOf") {
      compound = std::make_unique<AllOfCondition>();
    } else {
      compound = std::make_unique<AnyOfCondition>();
    }
    for(const auto& childValue : value["conditions"]) {
      auto child = ParseConditionJson(childValue, false);
      if(child.Status != ConditionParseStatus::Parsed) {
        return ConditionParseResult{};
      }
      if(type == "allOf") {
        static_cast<AllOfCondition&>(*compound).AddCondition(
          std::move(child.ConditionAst));
      } else {
        static_cast<AnyOfCondition&>(*compound).AddCondition(
          std::move(child.ConditionAst));
      }
    }

    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst = std::move(compound),
    };
  }

  if(type == "not") {
    if(!value.contains("condition")) {
      return ConditionParseResult{};
    }

    auto child = ParseConditionJson(value["condition"], false);
    if(child.Status != ConditionParseStatus::Parsed) {
      return ConditionParseResult{};
    }

    return ConditionParseResult{
      .Status = ConditionParseStatus::Parsed,
      .ConditionAst =
        std::make_unique<NotCondition>(std::move(child.ConditionAst)),
    };
  }

  return ConditionParseResult{};
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

std::unique_ptr<Condition>
ConstCondition::Clone() const
{
  return std::make_unique<ConstCondition>(m_Value);
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

std::unique_ptr<Condition>
EqualsCondition::Clone() const
{
  return std::make_unique<EqualsCondition>(m_Left, m_Right);
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

std::unique_ptr<Condition>
NotEqualsCondition::Clone() const
{
  return std::make_unique<NotEqualsCondition>(m_Left, m_Right);
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

std::unique_ptr<Condition>
InListCondition::Clone() const
{
  return std::make_unique<InListCondition>(m_Value, m_Values);
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

std::unique_ptr<Condition>
NotInListCondition::Clone() const
{
  return std::make_unique<NotInListCondition>(m_Value, m_Values);
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

std::unique_ptr<Condition>
MatchesCondition::Clone() const
{
  return std::make_unique<MatchesCondition>(m_Value, m_Pattern);
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

std::unique_ptr<Condition>
NotMatchesCondition::Clone() const
{
  return std::make_unique<NotMatchesCondition>(m_Value, m_Pattern);
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

std::unique_ptr<Condition>
AllOfCondition::Clone() const
{
  auto clone = std::make_unique<AllOfCondition>();
  for(const auto& child : m_Children) {
    clone->AddCondition(child->Clone());
  }
  return clone;
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

std::unique_ptr<Condition>
AnyOfCondition::Clone() const
{
  auto clone = std::make_unique<AnyOfCondition>();
  for(const auto& child : m_Children) {
    clone->AddCondition(child->Clone());
  }
  return clone;
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

std::unique_ptr<Condition>
NotCondition::Clone() const
{
  return std::make_unique<NotCondition>(m_Child->Clone());
}

ConditionParseResult
ParseConditionJson(const nlohmann::json& value)
{
  return ParseConditionJson(value, true);
}

}  // namespace nhc::preset_graph
