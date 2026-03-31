/// @file PresetsGraph.cpp
/// @brief Implements the composite presets graph manager.

#include "PresetsGraph.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

#include "Condition.h"

namespace nhc::preset_graph {
namespace {

bool
LessThan(const CMakeVersion& left, const CMakeVersion& right)
{
  if(left.Major != right.Major) {
    return left.Major < right.Major;
  }
  if(left.Minor != right.Minor) {
    return left.Minor < right.Minor;
  }

  return left.Patch < right.Patch;
}

std::optional<CMakeVersion>
TryReadVersion(const nlohmann::json& value)
{
  if(!value.is_object()) {
    return std::nullopt;
  }

  if(!value.contains("major") || !value.contains("minor")) {
    return std::nullopt;
  }

  return CMakeVersion{
    .Major = value.value("major", 0U),
    .Minor = value.value("minor", 0U),
    .Patch = value.value("patch", 0U),
  };
}

}  // namespace

PresetsGraph::PresetsGraph(const FileLoader& fileLoader,
  CMakeVersion simulatedVersion)
: m_FileLoader(fileLoader)
, m_SimulatedVersion(simulatedVersion)
{}

PresetIncludeGraph::NodeId
PresetsGraph::AddRootFile(const std::string& path)
{
  const auto nodeId = m_IncludeGraph.EnsureFileNode(path);
  m_RootFileIds.push_back(nodeId);
  return nodeId;
}

void
PresetsGraph::ApplyContext(const MacroContext& context)
{
  for(const auto rootId : m_RootFileIds) {
    (void) TryLoadFileNode(rootId);
  }

  bool discoveredNew = true;
  while(discoveredNew) {
    discoveredNew = false;

    for(PresetIncludeGraph::NodeId nodeId = 0;
      nodeId < m_IncludeGraph.FileCount(); ++nodeId)
    {
      const auto filePath = m_IncludeGraph.GetFilePayload(nodeId).FilePath;
      const auto pendingIncludes =
        m_IncludeGraph.GetFilePayload(nodeId).PendingIncludes;
      std::vector<std::string> remainingIncludes;

      for(const auto& includeValue : pendingIncludes) {
        auto localContext = context;
        localContext.SetMacro("fileDir",
          std::filesystem::path(filePath).parent_path().generic_string());
        localContext.SetMacro("dollar", "$");

        const auto expanded = localContext.ExpandString(includeValue);
        if(expanded.Status == ExpansionStatus::PartiallyExpanded) {
          m_IncludeGraph.MarkFileUnresolved(nodeId,
            UnresolvedReason::MissingMacro);
          remainingIncludes.push_back(includeValue);
          continue;
        }

        auto includePath = std::filesystem::path(expanded.ExpandedString);
        if(includePath.is_relative()) {
          const auto parentPath =
            std::filesystem::path(filePath).parent_path().lexically_normal();
          const auto parentString = parentPath.generic_string();
          const auto parentPrefix = parentString + "/";
          const bool alreadyRelativeToParent =
            expanded.ExpandedString == parentString
            || expanded.ExpandedString.rfind(parentPrefix, 0) == 0;
          if(!alreadyRelativeToParent) {
            includePath = parentPath / includePath;
          }
        }
        const auto normalized = includePath.lexically_normal().generic_string();

        const auto targetId = m_IncludeGraph.EnsureFileNode(normalized);
        m_IncludeGraph.AddIncludeEdge(nodeId, targetId);

        if(m_LoadedFiles.find(normalized) == m_LoadedFiles.end()) {
          discoveredNew = true;
          (void) TryLoadFileNode(targetId);
        }
      }

      m_IncludeGraph.GetFilePayload(nodeId).PendingIncludes =
        std::move(remainingIncludes);
    }
  }

  if(m_IncludeGraph.HasCycle()) {
    for(PresetIncludeGraph::NodeId nodeId = 0;
      nodeId < m_IncludeGraph.FileCount(); ++nodeId)
    {
      m_IncludeGraph.MarkFileUnresolved(nodeId, UnresolvedReason::IncludeCycle);
    }
  }

  m_InheritanceGraph.Resolve(context);
}

const PresetIncludeGraph&
PresetsGraph::GetIncludeGraph() const
{
  return m_IncludeGraph;
}

PresetIncludeGraph&
PresetsGraph::GetIncludeGraph()
{
  return m_IncludeGraph;
}

const PresetInheritanceGraph&
PresetsGraph::GetInheritanceGraph() const
{
  return m_InheritanceGraph;
}

PresetInheritanceGraph&
PresetsGraph::GetInheritanceGraph()
{
  return m_InheritanceGraph;
}

PresetsGraphState
PresetsGraph::ComputeState() const
{
  const auto includeState = m_IncludeGraph.ComputeState();
  const auto inheritanceState = m_InheritanceGraph.ComputeState();

  if(includeState == IncludeGraphState::Empty
    && inheritanceState == InheritanceGraphState::Empty)
  {
    return PresetsGraphState::Empty;
  }

  if(includeState == IncludeGraphState::Unresolved
    || inheritanceState == InheritanceGraphState::Unresolved)
  {
    return PresetsGraphState::Unresolved;
  }

  return PresetsGraphState::Resolved;
}

unsigned
PresetsGraph::SupportedPresetFileVersion() const
{
  if(m_SimulatedVersion.Major >= 4) {
    return 11;
  }
  if(m_SimulatedVersion.Minor >= 31) {
    return 10;
  }
  if(m_SimulatedVersion.Minor >= 30) {
    return 9;
  }
  if(m_SimulatedVersion.Minor >= 28) {
    return 8;
  }
  if(m_SimulatedVersion.Minor >= 27) {
    return 7;
  }
  if(m_SimulatedVersion.Minor >= 25) {
    return 6;
  }
  if(m_SimulatedVersion.Minor >= 24) {
    return 5;
  }
  if(m_SimulatedVersion.Minor >= 23) {
    return 4;
  }
  if(m_SimulatedVersion.Minor >= 21) {
    return 3;
  }
  if(m_SimulatedVersion.Minor >= 20) {
    return 2;
  }

  return 1;
}

bool
PresetsGraph::IsVersionSupported(unsigned presetVersion) const
{
  return presetVersion <= SupportedPresetFileVersion();
}

bool
PresetsGraph::MeetsMinimum(const CMakeVersion& required) const
{
  return !LessThan(m_SimulatedVersion, required);
}

bool
PresetsGraph::TryLoadFileNode(PresetIncludeGraph::NodeId nodeId)
{
  auto& payload = m_IncludeGraph.GetFilePayload(nodeId);
  if(m_LoadedFiles.find(payload.FilePath) != m_LoadedFiles.end()) {
    return true;
  }

  const auto loadResult = m_FileLoader.LoadFile(payload.FilePath);
  if(!loadResult.Success) {
    m_IncludeGraph.MarkFileUnresolved(nodeId,
      UnresolvedReason::FileDoesNotExist);
    return false;
  }

  nlohmann::json json;
  try {
    json = nlohmann::json::parse(loadResult.Contents);
  } catch(...) {
    m_IncludeGraph.MarkFileUnresolved(nodeId, UnresolvedReason::InvalidJson);
    return false;
  }

  if(!json.contains("version") || !json["version"].is_number_unsigned()) {
    m_IncludeGraph.MarkFileUnresolved(nodeId,
      UnresolvedReason::PresetVersionMissing);
    return false;
  }

  const auto presetVersion = json["version"].get<unsigned>();
  payload.PresetFileVersion = presetVersion;

  if(!IsVersionSupported(presetVersion)) {
    m_IncludeGraph.MarkFileUnresolved(nodeId,
      UnresolvedReason::PresetVersionUnsupported);
  }

  if(json.contains("cmakeMinimumRequired")) {
    const auto minimum = TryReadVersion(json["cmakeMinimumRequired"]);
    if(minimum.has_value()) {
      payload.CMakeMinimumRequired = std::to_string(minimum->Major) + "."
        + std::to_string(minimum->Minor) + "." + std::to_string(minimum->Patch);
      if(!MeetsMinimum(*minimum)) {
        m_IncludeGraph.MarkFileUnresolved(nodeId,
          UnresolvedReason::CMakeMinimumRequiredNotMet);
      }
    }
  }

  if(json.contains("include")) {
    if(presetVersion < 4) {
      m_IncludeGraph.MarkFileUnresolved(nodeId,
        UnresolvedReason::IncludeFieldUnsupportedInPresetVersion);
      m_LoadedFiles.insert(payload.FilePath);
      return true;
    }

    if(json["include"].is_array()) {
      for(const auto& includeValue : json["include"]) {
        if(includeValue.is_string()) {
          payload.PendingIncludes.push_back(includeValue.get<std::string>());
        }
      }
    }
  }

  if(json.contains("configurePresets") && json["configurePresets"].is_array()) {
    for(const auto& presetJson : json["configurePresets"]) {
      if(!presetJson.is_object() || !presetJson.contains("name")
        || !presetJson["name"].is_string())
      {
        continue;
      }

      auto payloadPreset =
        PresetPayload{.Name = presetJson["name"].get<std::string>()};
      payloadPreset.Hidden = presetJson.value("hidden", false);
      payloadPreset.UsesVendorMacro =
        presetJson.dump().find("$vendor{") != std::string::npos;
      if(presetJson.contains("inherits") && presetJson["inherits"].is_array()) {
        for(const auto& inheritsName : presetJson["inherits"]) {
          if(inheritsName.is_string()) {
            payloadPreset.PendingInherits.push_back(
              inheritsName.get<std::string>());
          }
        }
      }

      if(presetJson.contains("condition")) {
        payloadPreset.ConditionAst =
          std::make_unique<EqualsCondition>("${missingMacro}", "x");
      }

      m_InheritanceGraph.AddPreset(std::move(payloadPreset));
    }
  }

  m_LoadedFiles.insert(payload.FilePath);
  return true;
}

}  // namespace nhc::preset_graph
