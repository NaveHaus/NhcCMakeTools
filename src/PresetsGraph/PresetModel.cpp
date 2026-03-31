/// @file PresetModel.cpp
/// @brief Implements preset model resolution behavior.

#include "PresetModel.h"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace nhc::preset_graph {
namespace {

void
ApplyEnvironmentEntries(
  std::unordered_map<std::string, std::string>& destination,
  const std::unordered_map<std::string, std::string>& source)
{
  for(const auto& [name, value] : source) {
    destination[name] = value;
  }
}

void
ApplyEnvironmentEntries(
  std::unordered_map<std::string, std::string>& destination,
  const std::unordered_map<std::string, std::optional<std::string>>& source)
{
  for(const auto& [name, value] : source) {
    if(value.has_value()) {
      destination[name] = *value;
    } else {
      destination.erase(name);
    }
  }
}

std::vector<std::string>
ExtractEnvDependencies(const std::string& value)
{
  std::vector<std::string> dependencies;
  size_t index = 0;
  while(index < value.size()) {
    const auto marker = value.find("$env{", index);
    if(marker == std::string::npos) {
      break;
    }

    const auto close = value.find('}', marker + 5);
    if(close == std::string::npos) {
      break;
    }

    dependencies.push_back(value.substr(marker + 5, close - (marker + 5)));
    index = close + 1;
  }

  return dependencies;
}

bool
IsBuildType(PresetType type)
{
  return type == PresetType::Build || type == PresetType::Test
    || type == PresetType::Package;
}

}  // namespace

void
PresetModel::AddPreset(Preset preset)
{
  m_Presets[preset.Name] = std::move(preset);
}

const Preset&
PresetModel::GetPreset(const std::string& name) const
{
  return m_Presets.at(name);
}

PresetType
PresetModel::GetPresetType(const std::string& name) const
{
  return m_Presets.at(name).Type;
}

ResolvedPreset
PresetModel::ResolvePreset(const std::string& name,
  const MacroContext& context) const
{
  const auto raw = ResolveRawPreset(name);
  auto result = ResolvedPreset{};
  result.Type = raw.Type;
  result.Name = raw.Name;
  result.InstallDir = raw.InstallDir;
  result.EffectiveGenerator = raw.EffectiveGenerator;

  auto expandedValues = std::unordered_map<std::string, std::string>{};
  auto expandedStatusByKey = std::unordered_map<std::string, ExpansionStatus>{};
  auto visiting = std::unordered_set<std::string>{};
  auto visited = std::unordered_set<std::string>{};

  std::function<void(const std::string&)> resolveEnvironmentKey;
  resolveEnvironmentKey = [&](const std::string& key) {
    if(visited.contains(key) || !raw.RawEnvironment.contains(key)) {
      return;
    }

    if(visiting.contains(key)) {
      result.EnvironmentStatus = ExpansionStatus::PartiallyExpanded;
      result.Reason = UnresolvedReason::EnvironmentCycle;
      return;
    }

    visiting.insert(key);

    const auto& rawValue = raw.RawEnvironment.at(key);
    for(const auto& dependency : ExtractEnvDependencies(rawValue)) {
      if(raw.RawEnvironment.contains(dependency)) {
        resolveEnvironmentKey(dependency);
      }
    }

    auto localContext = context;
    localContext.SetMacro("presetName", raw.Name);
    if(raw.EffectiveGenerator.has_value()) {
      localContext.SetMacro("generator", *raw.EffectiveGenerator);
    }

    for(const auto& [envName, envValue] : expandedValues) {
      localContext.SetPresetEnvironmentValue(envName, envValue);
    }

    const auto expanded = localContext.ExpandString(rawValue);
    expandedValues[key] = expanded.ExpandedString;
    expandedStatusByKey[key] = expanded.Status;

    visiting.erase(key);
    visited.insert(key);
  };

  for(const auto& [key, _] : raw.RawEnvironment) {
    resolveEnvironmentKey(key);
  }

  for(const auto& [key, value] : expandedValues) {
    result.Environment[key] = value;
    if(expandedStatusByKey[key] == ExpansionStatus::PartiallyExpanded) {
      result.EnvironmentStatus = ExpansionStatus::PartiallyExpanded;
    }
  }

  return result;
}

PresetModel::RawResolvedPreset
PresetModel::ResolveRawPreset(const std::string& name) const
{
  const auto& preset = m_Presets.at(name);

  auto resolved = RawResolvedPreset{};
  resolved.Type = preset.Type;
  resolved.Name = preset.Name;

  for(auto it = preset.Inherits.rbegin(); it != preset.Inherits.rend(); ++it) {
    const auto parent = ResolveRawPreset(*it);
    if(parent.InstallDir.has_value()) {
      resolved.InstallDir = parent.InstallDir;
    }
    if(parent.EffectiveGenerator.has_value()) {
      resolved.EffectiveGenerator = parent.EffectiveGenerator;
    }
    ApplyEnvironmentEntries(resolved.RawEnvironment, parent.RawEnvironment);
  }

  if(IsBuildType(preset.Type) && preset.InheritConfigureEnvironment
    && preset.ConfigurePreset.has_value())
  {
    const auto configure = ResolveRawPreset(*preset.ConfigurePreset);
    ApplyEnvironmentEntries(resolved.RawEnvironment, configure.RawEnvironment);
  }

  ApplyEnvironmentEntries(resolved.RawEnvironment, preset.Environment);

  if(preset.InstallDir.has_value()) {
    resolved.InstallDir = preset.InstallDir;
  }
  if(preset.Generator.has_value()) {
    resolved.EffectiveGenerator = preset.Generator;
  }

  if(IsBuildType(preset.Type) && preset.ConfigurePreset.has_value()) {
    const auto configure = ResolveRawPreset(*preset.ConfigurePreset);
    resolved.EffectiveGenerator = configure.EffectiveGenerator;
  }

  return resolved;
}

}  // namespace nhc::preset_graph
