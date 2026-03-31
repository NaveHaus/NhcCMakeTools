/// @file MacroContext.cpp
/// @brief Implements macro expansion for preset strings.

#include "MacroContext.h"

#include <string_view>

namespace nhc::preset_graph {
namespace {

bool
StartsWithAt(const std::string& input, size_t index, std::string_view prefix)
{
  if(index + prefix.size() > input.size()) {
    return false;
  }

  return input.compare(index, prefix.size(), prefix) == 0;
}

bool
TryExtractToken(const std::string& input, size_t index, std::string_view prefix,
  std::string& token, std::string& name, size_t& nextIndex)
{
  if(!StartsWithAt(input, index, prefix)) {
    return false;
  }

  const size_t open = index + prefix.size();
  const size_t close = input.find('}', open);
  if(close == std::string::npos) {
    return false;
  }

  token = input.substr(index, close - index + 1);
  name = input.substr(open, close - open);
  nextIndex = close + 1;
  return true;
}

}  // namespace

void
MacroContext::SetMacro(std::string name, std::string value)
{
  m_Macros[std::move(name)] = std::move(value);
}

std::optional<std::string>
MacroContext::GetMacro(const std::string& name) const
{
  auto it = m_Macros.find(name);
  if(it == m_Macros.end()) {
    return std::nullopt;
  }

  return it->second;
}

void
MacroContext::SetPresetEnvironmentValue(std::string name, std::string value)
{
  m_PresetEnvironment[std::move(name)] = std::move(value);
}

void
MacroContext::SetParentEnvironmentValue(std::string name, std::string value)
{
  m_ParentEnvironment[std::move(name)] = std::move(value);
}

ExpansionResult
MacroContext::ExpandString(const std::string& input) const
{
  ExpansionResult result;
  result.ExpandedString.reserve(input.size());

  size_t index = 0;
  while(index < input.size()) {
    if(input[index] != '$') {
      result.ExpandedString.push_back(input[index]);
      ++index;
      continue;
    }

    std::string token;
    std::string name;
    size_t nextIndex = index + 1;
    bool handled = false;

    if(TryExtractToken(input, index, "${", token, name, nextIndex)) {
      auto it = m_Macros.find(name);
      if(it != m_Macros.end()) {
        result.ExpandedString.append(it->second);
      } else {
        result.ExpandedString.append(token);
        result.UnresolvedTokens.push_back(token);
      }
      handled = true;
    } else if(TryExtractToken(input, index, "$env{", token, name, nextIndex)) {
      auto it = m_PresetEnvironment.find(name);
      if(it != m_PresetEnvironment.end()) {
        result.ExpandedString.append(it->second);
      } else {
        auto parentIt = m_ParentEnvironment.find(name);
        if(parentIt != m_ParentEnvironment.end()) {
          result.ExpandedString.append(parentIt->second);
        } else {
          result.ExpandedString.append(token);
          result.UnresolvedTokens.push_back(token);
        }
      }
      handled = true;
    } else if(TryExtractToken(input, index, "$penv{", token, name, nextIndex)) {
      auto it = m_ParentEnvironment.find(name);
      if(it != m_ParentEnvironment.end()) {
        result.ExpandedString.append(it->second);
      } else {
        result.ExpandedString.append(token);
        result.UnresolvedTokens.push_back(token);
      }
      handled = true;
    }

    if(handled) {
      index = nextIndex;
      continue;
    }

    result.ExpandedString.push_back(input[index]);
    ++index;
  }

  result.Status = result.UnresolvedTokens.empty()
    ? ExpansionStatus::FullyExpanded
    : ExpansionStatus::PartiallyExpanded;
  return result;
}

}  // namespace nhc::preset_graph
