/// @file PresetModel.h
/// @brief Defines typed preset model and resolution behavior.

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Condition.h"
#include "IncludeGraph.h"
#include "MacroContext.h"

namespace nhc::preset_graph {

enum class PresetKind
{
  Configure,
  Build,
  Test,
  Package,
  Workflow
};

using PresetEnvironment =
  std::unordered_map<std::string, std::optional<std::string>>;

enum class PresetConditionState
{
  Absent,
  ExplicitNull,
  Expression
};

enum class ResolvedFieldStatus
{
  Unresolved,
  PartiallyResolved,
  FullyResolved
};

struct ResolvedField
{
  nlohmann::json Value;
  ResolvedFieldStatus Status = ResolvedFieldStatus::Unresolved;
};

class Preset
{
  public:
  virtual ~Preset() = default;

  const std::string& GetName() const;
  void SetName(std::string name);

  virtual PresetKind GetType() const = 0;

  bool GetHidden() const;
  void SetHidden(bool hidden);

  const std::vector<std::string>& GetInherits() const;
  void SetInherits(std::vector<std::string> inherits);

  const Condition* GetCondition() const;
  void SetCondition(std::unique_ptr<Condition> condition);
  PresetConditionState GetConditionState() const;
  void SetConditionExplicitNull();
  void ClearCondition();

  const PresetEnvironment& GetEnvironment() const;
  void SetEnvironment(PresetEnvironment environment);

  const nlohmann::json& GetRawJson() const;
  void SetRawJson(nlohmann::json rawJson);

  const std::unordered_map<std::string, ResolvedField>& GetResolvedFields()
    const;
  void ClearResolvedFields();
  void SetResolvedField(std::string fieldName, ResolvedField field);

  bool GetIsUnresolved() const;
  const std::optional<UnresolvedReason>& GetReason() const;
  void MarkUnresolved(UnresolvedReason reason);
  void ClearUnresolved();

  protected:
  std::string m_Name;
  bool m_Hidden = false;
  std::vector<std::string> m_Inherits;
  PresetConditionState m_ConditionState = PresetConditionState::Absent;
  std::unique_ptr<Condition> m_Condition;
  PresetEnvironment m_Environment;
  nlohmann::json m_RawJson = nlohmann::json::object();
  std::unordered_map<std::string, ResolvedField> m_ResolvedFields;
  bool m_IsUnresolved = false;
  std::optional<UnresolvedReason> m_Reason;
};

class ConfigurePreset : public Preset
{
  public:
  PresetKind GetType() const override;

  const std::optional<std::string>& GetInstallDir() const;
  void SetInstallDir(std::optional<std::string> installDir);

  const std::optional<std::string>& GetGenerator() const;
  void SetGenerator(std::optional<std::string> generator);

  private:
  std::optional<std::string> m_InstallDir;
  std::optional<std::string> m_Generator;
};

class ConfigureConsumerPreset : public Preset
{
  public:
  const std::optional<std::string>& GetConfigurePreset() const;
  void SetConfigurePreset(std::optional<std::string> configurePreset);

  bool GetInheritConfigureEnvironment() const;
  void SetInheritConfigureEnvironment(bool inheritConfigureEnvironment);

  protected:
  std::optional<std::string> m_ConfigurePreset;
  bool m_InheritConfigureEnvironment = true;
};

class BuildPreset : public ConfigureConsumerPreset
{
  public:
  PresetKind GetType() const override;
};

class TestPreset : public ConfigureConsumerPreset
{
  public:
  PresetKind GetType() const override;
};

class PackagePreset : public ConfigureConsumerPreset
{
  public:
  PresetKind GetType() const override;
};

enum class WorkflowStepType
{
  Configure,
  Build,
  Test,
  Package
};

struct WorkflowStep
{
  WorkflowStepType Type = WorkflowStepType::Configure;
  std::string Name;
};

class WorkflowPreset : public Preset
{
  public:
  PresetKind GetType() const override;

  const std::vector<WorkflowStep>& GetSteps() const;
  void SetSteps(std::vector<WorkflowStep> steps);

  private:
  using Preset::ClearCondition;
  using Preset::GetCondition;
  using Preset::GetConditionState;
  using Preset::GetEnvironment;
  using Preset::GetHidden;
  using Preset::GetInherits;
  using Preset::SetCondition;
  using Preset::SetConditionExplicitNull;
  using Preset::SetEnvironment;
  using Preset::SetHidden;
  using Preset::SetInherits;

  std::vector<WorkflowStep> m_Steps;
};

/// Resolved view used by graph resolution.
struct ResolvedPreset
{
  PresetKind Type = PresetKind::Configure;
  std::string Name;

  std::optional<std::string> InstallDir;
  std::optional<std::string> EffectiveGenerator;

  std::unordered_map<std::string, std::string> Environment;
  ExpansionStatus EnvironmentStatus = ExpansionStatus::FullyExpanded;
  std::optional<UnresolvedReason> Reason;
};

/// Preset model with inheritance and environment resolution.
class PresetModel
{
  public:
  void AddPreset(std::unique_ptr<Preset> preset);
  void RemovePreset(const std::string& name);

  const Preset& GetPreset(const std::string& name) const;

  template<typename TPreset>
  const TPreset* GetPreset(const std::string& name) const
  {
    const auto& preset = GetPreset(name);
    return dynamic_cast<const TPreset*>(&preset);
  }

  PresetKind GetPresetKind(const std::string& name) const;

  std::vector<const Preset*> GetPresets() const;

  const Condition* ResolveCondition(const std::string& name) const;

  void RefreshResolvedState(const std::string& name,
    const MacroContext& context = MacroContext{});

  ResolvedPreset ResolvePreset(const std::string& name,
    const MacroContext& context = MacroContext{});

  private:
  const Condition* ResolveCondition(const std::string& name,
    std::unordered_set<std::string>& visiting) const;

  struct MergedPresetFields
  {
    PresetKind Type = PresetKind::Configure;
    std::string Name;

    std::optional<std::string> InstallDir;
    std::optional<std::string> EffectiveGenerator;

    std::unordered_map<std::string, std::string> RawEnvironment;
    std::unordered_map<std::string, nlohmann::json> RawFields;
  };

  MergedPresetFields ResolveMergedFields(const std::string& name) const;

  std::unordered_map<std::string, std::unique_ptr<Preset>> m_Presets;
};

}  // namespace nhc::preset_graph
