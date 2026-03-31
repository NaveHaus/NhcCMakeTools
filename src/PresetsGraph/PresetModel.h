/// @file PresetModel.h
/// @brief Defines typed preset model and resolution behavior.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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

  const PresetEnvironment& GetEnvironment() const;
  void SetEnvironment(PresetEnvironment environment);

  protected:
  std::string m_Name;
  bool m_Hidden = false;
  std::vector<std::string> m_Inherits;
  std::unique_ptr<Condition> m_Condition;
  PresetEnvironment m_Environment;
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
  using Preset::GetCondition;
  using Preset::GetEnvironment;
  using Preset::GetHidden;
  using Preset::GetInherits;
  using Preset::SetCondition;
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

  const Preset& GetPreset(const std::string& name) const;

  template<typename TPreset>
  const TPreset* GetPreset(const std::string& name) const
  {
    const auto& preset = GetPreset(name);
    return dynamic_cast<const TPreset*>(&preset);
  }

  PresetKind GetPresetKind(const std::string& name) const;

  ResolvedPreset ResolvePreset(const std::string& name,
    const MacroContext& context = MacroContext{}) const;

  private:
  struct RawResolvedPreset
  {
    PresetKind Type = PresetKind::Configure;
    std::string Name;

    std::optional<std::string> InstallDir;
    std::optional<std::string> EffectiveGenerator;

    std::unordered_map<std::string, std::string> RawEnvironment;
  };

  RawResolvedPreset ResolveRawPreset(const std::string& name) const;

  std::unordered_map<std::string, std::unique_ptr<Preset>> m_Presets;
};

}  // namespace nhc::preset_graph
