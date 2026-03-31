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
IsBuildType(PresetKind type)
{
  return type == PresetKind::Build || type == PresetKind::Test
    || type == PresetKind::Package;
}

const ConfigurePreset*
AsConfigurePreset(const Preset& preset)
{
  return dynamic_cast<const ConfigurePreset*>(&preset);
}

const ConfigureConsumerPreset*
AsConfigureConsumerPreset(const Preset& preset)
{
  return dynamic_cast<const ConfigureConsumerPreset*>(&preset);
}

}  // namespace

const std::string&
Preset::GetName() const
{
  return m_Name;
}

void
Preset::SetName(std::string name)
{
  m_Name = std::move(name);
}

bool
Preset::GetHidden() const
{
  return m_Hidden;
}

void
Preset::SetHidden(bool hidden)
{
  m_Hidden = hidden;
}

const std::vector<std::string>&
Preset::GetInherits() const
{
  return m_Inherits;
}

void
Preset::SetInherits(std::vector<std::string> inherits)
{
  m_Inherits = std::move(inherits);
}

const Condition*
Preset::GetCondition() const
{
  return m_Condition.get();
}

void
Preset::SetCondition(std::unique_ptr<Condition> condition)
{
  m_Condition = std::move(condition);
}

const PresetEnvironment&
Preset::GetEnvironment() const
{
  return m_Environment;
}

void
Preset::SetEnvironment(PresetEnvironment environment)
{
  m_Environment = std::move(environment);
}

PresetKind
ConfigurePreset::GetType() const
{
  return PresetKind::Configure;
}

const std::optional<std::string>&
ConfigurePreset::GetInstallDir() const
{
  return m_InstallDir;
}

void
ConfigurePreset::SetInstallDir(std::optional<std::string> installDir)
{
  m_InstallDir = std::move(installDir);
}

const std::optional<std::string>&
ConfigurePreset::GetGenerator() const
{
  return m_Generator;
}

void
ConfigurePreset::SetGenerator(std::optional<std::string> generator)
{
  m_Generator = std::move(generator);
}

const std::optional<std::string>&
ConfigureConsumerPreset::GetConfigurePreset() const
{
  return m_ConfigurePreset;
}

void
ConfigureConsumerPreset::SetConfigurePreset(
  std::optional<std::string> configurePreset)
{
  m_ConfigurePreset = std::move(configurePreset);
}

bool
ConfigureConsumerPreset::GetInheritConfigureEnvironment() const
{
  return m_InheritConfigureEnvironment;
}

void
ConfigureConsumerPreset::SetInheritConfigureEnvironment(
  bool inheritConfigureEnvironment)
{
  m_InheritConfigureEnvironment = inheritConfigureEnvironment;
}

PresetKind
BuildPreset::GetType() const
{
  return PresetKind::Build;
}

PresetKind
TestPreset::GetType() const
{
  return PresetKind::Test;
}

PresetKind
PackagePreset::GetType() const
{
  return PresetKind::Package;
}

PresetKind
WorkflowPreset::GetType() const
{
  return PresetKind::Workflow;
}

const std::vector<WorkflowStep>&
WorkflowPreset::GetSteps() const
{
  return m_Steps;
}

void
WorkflowPreset::SetSteps(std::vector<WorkflowStep> steps)
{
  m_Steps = std::move(steps);
}

void
PresetModel::AddPreset(std::unique_ptr<Preset> preset)
{
  m_Presets[preset->GetName()] = std::move(preset);
}

const Preset&
PresetModel::GetPreset(const std::string& name) const
{
  return *m_Presets.at(name);
}

PresetKind
PresetModel::GetPresetKind(const std::string& name) const
{
  return m_Presets.at(name)->GetType();
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
  const auto& preset = *m_Presets.at(name);

  auto resolved = RawResolvedPreset{};
  resolved.Type = preset.GetType();
  resolved.Name = preset.GetName();

  for(auto it = preset.GetInherits().rbegin();
    it != preset.GetInherits().rend(); ++it)
  {
    const auto parent = ResolveRawPreset(*it);
    if(parent.InstallDir.has_value()) {
      resolved.InstallDir = parent.InstallDir;
    }
    if(parent.EffectiveGenerator.has_value()) {
      resolved.EffectiveGenerator = parent.EffectiveGenerator;
    }
    ApplyEnvironmentEntries(resolved.RawEnvironment, parent.RawEnvironment);
  }

  if(const auto* buildPreset = AsConfigureConsumerPreset(preset);
    buildPreset != nullptr && buildPreset->GetInheritConfigureEnvironment()
    && buildPreset->GetConfigurePreset().has_value())
  {
    const auto configure = ResolveRawPreset(*buildPreset->GetConfigurePreset());
    ApplyEnvironmentEntries(resolved.RawEnvironment, configure.RawEnvironment);
  }

  ApplyEnvironmentEntries(resolved.RawEnvironment, preset.GetEnvironment());

  if(const auto* configurePreset = AsConfigurePreset(preset);
    configurePreset != nullptr)
  {
    if(configurePreset->GetInstallDir().has_value()) {
      resolved.InstallDir = configurePreset->GetInstallDir();
    }
    if(configurePreset->GetGenerator().has_value()) {
      resolved.EffectiveGenerator = configurePreset->GetGenerator();
    }
  }

  if(const auto* buildPreset = AsConfigureConsumerPreset(preset);
    buildPreset != nullptr && buildPreset->GetConfigurePreset().has_value())
  {
    const auto configure = ResolveRawPreset(*buildPreset->GetConfigurePreset());
    resolved.EffectiveGenerator = configure.EffectiveGenerator;
  }

  return resolved;
}

}  // namespace nhc::preset_graph
