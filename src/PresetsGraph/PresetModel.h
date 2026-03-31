/// @file PresetModel.h
/// @brief Defines typed preset model and resolution behavior.

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "IncludeGraph.h"
#include "MacroContext.h"

namespace nhc::preset_graph {

enum class PresetType
{
  Configure,
  Build,
  Test,
  Package,
  Workflow
};

/// Raw preset representation for model resolution.
struct Preset
{
  std::string Name;
  PresetType Type = PresetType::Configure;
  std::vector<std::string> Inherits;

  std::optional<std::string> InstallDir;
  std::optional<std::string> Generator;

  std::optional<std::string> ConfigurePreset;
  bool InheritConfigureEnvironment = false;

  std::unordered_map<std::string, std::optional<std::string>> Environment;
};

/// Resolved view used by graph resolution.
struct ResolvedPreset
{
  PresetType Type = PresetType::Configure;
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
  void AddPreset(Preset preset);

  const Preset& GetPreset(const std::string& name) const;

  PresetType GetPresetType(const std::string& name) const;

  ResolvedPreset ResolvePreset(const std::string& name,
    const MacroContext& context = MacroContext{}) const;

  private:
  struct RawResolvedPreset
  {
    PresetType Type = PresetType::Configure;
    std::string Name;

    std::optional<std::string> InstallDir;
    std::optional<std::string> EffectiveGenerator;

    std::unordered_map<std::string, std::string> RawEnvironment;
  };

  RawResolvedPreset ResolveRawPreset(const std::string& name) const;

  std::unordered_map<std::string, Preset> m_Presets;
};

}  // namespace nhc::preset_graph
