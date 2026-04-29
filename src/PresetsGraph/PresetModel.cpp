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

ResolvedFieldStatus
StatusForExpansion(ExpansionStatus status)
{
  return status == ExpansionStatus::FullyExpanded
    ? ResolvedFieldStatus::FullyResolved
    : ResolvedFieldStatus::PartiallyResolved;
}

bool
IsScalarPresetField(const std::string& fieldName)
{
  return fieldName == "generator" || fieldName == "installDir"
    || fieldName == "binaryDir" || fieldName == "toolchainFile";
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
  m_ConditionState = m_Condition
    ? PresetConditionState::Expression
    : PresetConditionState::Absent;
}

PresetConditionState
Preset::GetConditionState() const
{
  return m_ConditionState;
}

void
Preset::SetConditionExplicitNull()
{
  m_Condition.reset();
  m_ConditionState = PresetConditionState::ExplicitNull;
}

void
Preset::ClearCondition()
{
  m_Condition.reset();
  m_ConditionState = PresetConditionState::Absent;
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

const nlohmann::json&
Preset::GetRawJson() const
{
  return m_RawJson;
}

void
Preset::SetRawJson(nlohmann::json rawJson)
{
  m_RawJson = std::move(rawJson);
}

const std::unordered_map<std::string, ResolvedField>&
Preset::GetResolvedFields() const
{
  return m_ResolvedFields;
}

void
Preset::ClearResolvedFields()
{
  m_ResolvedFields.clear();
}

void
Preset::SetResolvedField(std::string fieldName, ResolvedField field)
{
  m_ResolvedFields[std::move(fieldName)] = std::move(field);
}

bool
Preset::GetIsUnresolved() const
{
  return m_IsUnresolved;
}

const std::optional<UnresolvedReason>&
Preset::GetReason() const
{
  return m_Reason;
}

void
Preset::MarkUnresolved(UnresolvedReason reason)
{
  m_IsUnresolved = true;
  m_Reason = reason;
}

void
Preset::ClearUnresolved()
{
  m_IsUnresolved = false;
  m_Reason.reset();
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

void
PresetModel::RemovePreset(const std::string& name)
{
  m_Presets.erase(name);
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

std::vector<const Preset*>
PresetModel::GetPresets() const
{
  std::vector<const Preset*> presets;
  presets.reserve(m_Presets.size());
  for(const auto& [_, preset] : m_Presets) {
    presets.push_back(preset.get());
  }
  return presets;
}

const Condition*
PresetModel::ResolveCondition(const std::string& name) const
{
  const auto& preset = *m_Presets.at(name);
  if(preset.GetConditionState() == PresetConditionState::Expression) {
    return preset.GetCondition();
  }
  if(preset.GetConditionState() == PresetConditionState::ExplicitNull) {
    return nullptr;
  }

  const Condition* inherited = nullptr;
  for(auto it = preset.GetInherits().rbegin();
    it != preset.GetInherits().rend(); ++it)
  {
    if(m_Presets.contains(*it)) {
      if(const auto* parentCondition = ResolveCondition(*it);
        parentCondition != nullptr)
      {
        inherited = parentCondition;
      }
    }
  }

  return inherited;
}

void
PresetModel::RefreshResolvedState(const std::string& name,
  const MacroContext& context)
{
  const auto merged = ResolveMergedFields(name);
  auto& preset = *m_Presets.at(name);
  preset.ClearResolvedFields();

  auto localContext = context;
  localContext.SetMacro("presetName", merged.Name);
  if(merged.EffectiveGenerator.has_value()) {
    localContext.SetMacro("generator", *merged.EffectiveGenerator);
  }

  for(const auto& [fieldName, fieldValue] : merged.RawFields) {
    if(fieldValue.is_string() && IsScalarPresetField(fieldName)) {
      const auto expanded =
        localContext.ExpandString(fieldValue.get<std::string>());
      preset.SetResolvedField(fieldName,
        ResolvedField{.Value = expanded.ExpandedString,
          .Status = StatusForExpansion(expanded.Status)});
      continue;
    }

    preset.SetResolvedField(fieldName,
      ResolvedField{
        .Value = fieldValue, .Status = ResolvedFieldStatus::FullyResolved});
  }
}

ResolvedPreset
PresetModel::ResolvePreset(const std::string& name, const MacroContext& context)
{
  const auto raw = ResolveMergedFields(name);
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

PresetModel::MergedPresetFields
PresetModel::ResolveMergedFields(const std::string& name) const
{
  const auto& preset = *m_Presets.at(name);

  auto resolved = MergedPresetFields{};
  resolved.Type = preset.GetType();
  resolved.Name = preset.GetName();

  for(auto it = preset.GetInherits().rbegin();
    it != preset.GetInherits().rend(); ++it)
  {
    if(!m_Presets.contains(*it)) {
      continue;
    }

    const auto parent = ResolveMergedFields(*it);
    if(parent.InstallDir.has_value()) {
      resolved.InstallDir = parent.InstallDir;
    }
    if(parent.EffectiveGenerator.has_value()) {
      resolved.EffectiveGenerator = parent.EffectiveGenerator;
    }
    ApplyEnvironmentEntries(resolved.RawEnvironment, parent.RawEnvironment);
    for(const auto& [fieldName, fieldValue] : parent.RawFields) {
      resolved.RawFields[fieldName] = fieldValue;
    }
  }

  if(const auto* buildPreset = AsConfigureConsumerPreset(preset);
    buildPreset != nullptr && buildPreset->GetInheritConfigureEnvironment()
    && buildPreset->GetConfigurePreset().has_value())
  {
    if(m_Presets.contains(*buildPreset->GetConfigurePreset())) {
      const auto configure =
        ResolveMergedFields(*buildPreset->GetConfigurePreset());
      ApplyEnvironmentEntries(resolved.RawEnvironment,
        configure.RawEnvironment);
    }
  }

  ApplyEnvironmentEntries(resolved.RawEnvironment, preset.GetEnvironment());
  if(preset.GetRawJson().is_object()) {
    for(const auto& [fieldName, fieldValue] : preset.GetRawJson().items()) {
      resolved.RawFields[fieldName] = fieldValue;
    }
  }

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
    if(m_Presets.contains(*buildPreset->GetConfigurePreset())) {
      const auto configure =
        ResolveMergedFields(*buildPreset->GetConfigurePreset());
      resolved.EffectiveGenerator = configure.EffectiveGenerator;
    }
  }

  return resolved;
}

}  // namespace nhc::preset_graph
