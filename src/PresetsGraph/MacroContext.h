/// @file MacroContext.h
/// @brief Provides a macro expansion context for preset strings.

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace nhc::preset_graph {

/// Describes the macro expansion status.
enum class ExpansionStatus
{
  FullyExpanded,
  PartiallyExpanded
};

/// Represents the result of expanding a string with macros.
struct ExpansionResult
{
  std::string ExpandedString;
  ExpansionStatus Status = ExpansionStatus::FullyExpanded;
  std::vector<std::string> UnresolvedTokens;
};

/// Provides macro and environment values for string expansion.
class MacroContext
{
  public:
  /// Sets a macro value.
  void SetMacro(std::string name, std::string value);

  /// Retrieves a macro value if present.
  std::optional<std::string> GetMacro(const std::string& name) const;

  /// Sets a preset environment variable value.
  void SetPresetEnvironmentValue(std::string name, std::string value);

  /// Sets a parent environment variable value.
  void SetParentEnvironmentValue(std::string name, std::string value);

  /// Expands macros in the provided string.
  ExpansionResult ExpandString(const std::string& input) const;

  private:
  std::unordered_map<std::string, std::string> m_Macros;
  std::unordered_map<std::string, std::string> m_PresetEnvironment;
  std::unordered_map<std::string, std::string> m_ParentEnvironment;
};

}  // namespace nhc::preset_graph
