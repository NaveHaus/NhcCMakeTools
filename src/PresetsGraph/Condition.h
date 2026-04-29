/// @file Condition.h
/// @brief Defines condition AST nodes for preset evaluation.

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "MacroContext.h"

namespace nhc::preset_graph {

/// Tri-state result for condition evaluation.
enum class ConditionResult
{
  True,
  False,
  Unknown
};

/// Base class for all condition AST nodes.
class Condition
{
  public:
  virtual ~Condition() = default;

  /// Evaluates this condition against the supplied macro context.
  virtual ConditionResult Evaluate(const MacroContext& context) const = 0;

  /// Creates an equivalent condition node.
  virtual std::unique_ptr<Condition> Clone() const = 0;
};

/// Constant boolean condition node.
class ConstCondition final : public Condition
{
  public:
  explicit ConstCondition(bool value);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  bool m_Value;
};

/// Equality condition node.
class EqualsCondition final : public Condition
{
  public:
  EqualsCondition(std::string left, std::string right);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Left;
  std::string m_Right;
};

/// Inequality condition node.
class NotEqualsCondition final : public Condition
{
  public:
  NotEqualsCondition(std::string left, std::string right);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Left;
  std::string m_Right;
};

/// List membership condition node.
class InListCondition final : public Condition
{
  public:
  InListCondition(std::string value, std::vector<std::string> values);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Value;
  std::vector<std::string> m_Values;
};

/// Negated list membership condition node.
class NotInListCondition final : public Condition
{
  public:
  NotInListCondition(std::string value, std::vector<std::string> values);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Value;
  std::vector<std::string> m_Values;
};

/// Regular expression match condition node.
class MatchesCondition final : public Condition
{
  public:
  MatchesCondition(std::string value, std::string pattern);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Value;
  std::string m_Pattern;
};

/// Negated regular expression match condition node.
class NotMatchesCondition final : public Condition
{
  public:
  NotMatchesCondition(std::string value, std::string pattern);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::string m_Value;
  std::string m_Pattern;
};

/// Logical all-of condition node.
class AllOfCondition final : public Condition
{
  public:
  AllOfCondition() = default;

  void AddCondition(std::unique_ptr<Condition> condition);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::vector<std::unique_ptr<Condition>> m_Children;
};

/// Logical any-of condition node.
class AnyOfCondition final : public Condition
{
  public:
  AnyOfCondition() = default;

  void AddCondition(std::unique_ptr<Condition> condition);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::vector<std::unique_ptr<Condition>> m_Children;
};

/// Logical not condition node.
class NotCondition final : public Condition
{
  public:
  explicit NotCondition(std::unique_ptr<Condition> child);

  ConditionResult Evaluate(const MacroContext& context) const override;
  std::unique_ptr<Condition> Clone() const override;

  private:
  std::unique_ptr<Condition> m_Child;
};

enum class ConditionParseStatus
{
  Parsed,
  ExplicitNull,
  Invalid
};

struct ConditionParseResult
{
  ConditionParseStatus Status = ConditionParseStatus::Invalid;
  std::unique_ptr<Condition> ConditionAst;
};

/// Parses a CMake preset condition JSON value.
ConditionParseResult ParseConditionJson(const nlohmann::json& value);

}  // namespace nhc::preset_graph
