/// @file PresetsGraph.cpp
/// @brief Implements the composite presets graph manager.

#include "PresetsGraph.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
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

void
ResetFileLoadState(FilePayload& payload)
{
  payload.IsUnresolved = false;
  payload.Reason = std::nullopt;
}

std::vector<std::string>
ReadStringOrStringArray(const nlohmann::json& value)
{
  if(value.is_string()) {
    return {value.get<std::string>()};
  }

  std::vector<std::string> result;
  if(!value.is_array()) {
    return result;
  }

  for(const auto& entry : value) {
    if(entry.is_string()) {
      result.push_back(entry.get<std::string>());
    }
  }
  return result;
}

PresetEnvironment
ReadEnvironment(const nlohmann::json& value)
{
  PresetEnvironment environment;
  if(!value.is_object()) {
    return environment;
  }

  for(const auto& [name, entry] : value.items()) {
    if(entry.is_string()) {
      environment[name] = entry.get<std::string>();
    } else if(entry.is_null()) {
      environment[name] = std::nullopt;
    }
  }
  return environment;
}

std::optional<WorkflowStepType>
TryReadWorkflowStepType(const nlohmann::json& value)
{
  if(!value.is_string()) {
    return std::nullopt;
  }

  const auto type = value.get<std::string>();
  if(type == "configure") {
    return WorkflowStepType::Configure;
  }
  if(type == "build") {
    return WorkflowStepType::Build;
  }
  if(type == "test") {
    return WorkflowStepType::Test;
  }
  if(type == "package") {
    return WorkflowStepType::Package;
  }

  return std::nullopt;
}

void
ApplyCommonPresetFields(Preset& preset, const nlohmann::json& presetJson)
{
  preset.SetName(presetJson["name"].get<std::string>());
  preset.SetRawJson(presetJson);
  preset.SetHidden(presetJson.value("hidden", false));

  if(presetJson.contains("inherits")) {
    preset.SetInherits(ReadStringOrStringArray(presetJson["inherits"]));
  }

  if(presetJson.contains("environment")) {
    preset.SetEnvironment(ReadEnvironment(presetJson["environment"]));
  }

  if(presetJson.contains("condition")) {
    auto condition = ParseConditionJson(presetJson["condition"]);
    if(condition.Status == ConditionParseStatus::Parsed) {
      preset.SetCondition(std::move(condition.ConditionAst));
    } else if(condition.Status == ConditionParseStatus::ExplicitNull) {
      preset.SetConditionExplicitNull();
    } else {
      preset.MarkUnresolved(UnresolvedReason::InvalidCondition);
    }
  }
}

std::unique_ptr<Preset>
CreatePresetFromJson(PresetKind kind, const nlohmann::json& presetJson)
{
  if(!presetJson.is_object() || !presetJson.contains("name")
    || !presetJson["name"].is_string())
  {
    return nullptr;
  }

  if(kind == PresetKind::Configure) {
    auto preset = std::make_unique<ConfigurePreset>();
    ApplyCommonPresetFields(*preset, presetJson);
    if(presetJson.contains("installDir")
      && presetJson["installDir"].is_string())
    {
      preset->SetInstallDir(presetJson["installDir"].get<std::string>());
    }
    if(presetJson.contains("generator") && presetJson["generator"].is_string())
    {
      preset->SetGenerator(presetJson["generator"].get<std::string>());
    }
    return preset;
  }

  if(kind == PresetKind::Build || kind == PresetKind::Test
    || kind == PresetKind::Package)
  {
    std::unique_ptr<ConfigureConsumerPreset> preset;
    if(kind == PresetKind::Build) {
      preset = std::make_unique<BuildPreset>();
    } else if(kind == PresetKind::Test) {
      preset = std::make_unique<TestPreset>();
    } else {
      preset = std::make_unique<PackagePreset>();
    }

    ApplyCommonPresetFields(*preset, presetJson);
    if(presetJson.contains("configurePreset")
      && presetJson["configurePreset"].is_string())
    {
      preset->SetConfigurePreset(
        presetJson["configurePreset"].get<std::string>());
    }
    preset->SetInheritConfigureEnvironment(
      presetJson.value("inheritConfigureEnvironment", true));
    return preset;
  }

  auto preset = std::make_unique<WorkflowPreset>();
  preset->SetName(presetJson["name"].get<std::string>());
  preset->SetRawJson(presetJson);

  std::vector<WorkflowStep> steps;
  if(presetJson.contains("steps") && presetJson["steps"].is_array()) {
    for(const auto& stepJson : presetJson["steps"]) {
      if(!stepJson.is_object() || !stepJson.contains("type")
        || !stepJson.contains("name") || !stepJson["name"].is_string())
      {
        continue;
      }

      const auto type = TryReadWorkflowStepType(stepJson["type"]);
      if(type.has_value()) {
        steps.push_back(WorkflowStep{
          .Type = *type,
          .Name = stepJson["name"].get<std::string>(),
        });
      }
    }
  }
  preset->SetSteps(std::move(steps));
  return preset;
}

void
IngestPresetArray(PresetModel& model, std::vector<std::string>& names,
  const nlohmann::json& root, std::string_view arrayName, PresetKind kind)
{
  const auto key = std::string{arrayName};
  if(!root.contains(key) || !root[key].is_array()) {
    return;
  }

  for(const auto& presetJson : root[key]) {
    auto preset = CreatePresetFromJson(kind, presetJson);
    if(preset == nullptr) {
      continue;
    }
    names.push_back(preset->GetName());
    model.AddPreset(std::move(preset));
  }
}

bool
HasVendorMacro(const nlohmann::json& value)
{
  return value.dump().find("$vendor{") != std::string::npos;
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
  m_LoadedFiles.clear();
  m_PresetNamesByFile.clear();
  m_PresetModel = PresetModel{};
  m_InheritanceGraph = PresetInheritanceGraph{};
  m_WorkflowDiagnostics.clear();

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
        const auto normalized =
          std::filesystem::absolute(includePath)
            .lexically_normal()
            .generic_string();

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

  RefreshInheritanceGraph();
  ValidateWorkflowPresets();
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

const PresetModel&
PresetsGraph::GetPresetModel() const
{
  return m_PresetModel;
}

PresetModel&
PresetsGraph::GetPresetModel()
{
  return m_PresetModel;
}

const std::vector<WorkflowDiagnostic>&
PresetsGraph::GetWorkflowDiagnostics() const
{
  return m_WorkflowDiagnostics;
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
  if(m_SimulatedVersion.Major > 4
    || (m_SimulatedVersion.Major == 4 && m_SimulatedVersion.Minor >= 3))
  {
    return 11;
  }
  if(m_SimulatedVersion.Major == 4) {
    return 10;
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
  const auto filePath = m_IncludeGraph.GetFilePayload(nodeId).FilePath;
  if(m_LoadedFiles.find(filePath) != m_LoadedFiles.end()) {
    return true;
  }

  m_IncludeGraph.GetFilePayload(nodeId).PendingIncludes.clear();
  ResetFileLoadState(m_IncludeGraph.GetFilePayload(nodeId));

  const auto loadResult = m_FileLoader.LoadFile(filePath);
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
  m_IncludeGraph.GetFilePayload(nodeId).PresetFileVersion = presetVersion;

  if(!IsVersionSupported(presetVersion)) {
    m_IncludeGraph.MarkFileUnresolved(nodeId,
      UnresolvedReason::PresetVersionUnsupported);
  }

  if(json.contains("cmakeMinimumRequired")) {
    const auto minimum = TryReadVersion(json["cmakeMinimumRequired"]);
    if(minimum.has_value()) {
      m_IncludeGraph.GetFilePayload(nodeId).CMakeMinimumRequired =
        std::to_string(minimum->Major) + "." + std::to_string(minimum->Minor)
        + "." + std::to_string(minimum->Patch);
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
      m_LoadedFiles.insert(filePath);
      return true;
    }

    if(json["include"].is_array()) {
      for(const auto& includeValue : json["include"]) {
        if(includeValue.is_string()) {
          m_IncludeGraph.GetFilePayload(nodeId).PendingIncludes.push_back(
            includeValue.get<std::string>());
        }
      }
    }
  }

  const auto currentPath = std::filesystem::path(filePath);
  if(currentPath.filename() == "CMakeUserPresets.json") {
    auto sibling = currentPath.parent_path() / "CMakePresets.json";
    sibling = std::filesystem::absolute(sibling).lexically_normal();
    const auto siblingPath = sibling.generic_string();
    const auto siblingLoadResult = m_FileLoader.LoadFile(siblingPath);
    if(siblingLoadResult.Success) {
      const auto siblingId = m_IncludeGraph.EnsureFileNode(siblingPath);
      m_IncludeGraph.AddIncludeEdge(nodeId, siblingId);
      (void) TryLoadFileNode(siblingId);
    }
  }

  for(const auto& presetName : m_PresetNamesByFile[filePath]) {
    m_PresetModel.RemovePreset(presetName);
  }
  auto publishedNames = std::vector<std::string>{};
  IngestPresetArray(m_PresetModel, publishedNames, json, "configurePresets",
    PresetKind::Configure);
  IngestPresetArray(m_PresetModel, publishedNames, json, "buildPresets",
    PresetKind::Build);
  IngestPresetArray(m_PresetModel, publishedNames, json, "testPresets",
    PresetKind::Test);
  IngestPresetArray(m_PresetModel, publishedNames, json, "packagePresets",
    PresetKind::Package);
  IngestPresetArray(m_PresetModel, publishedNames, json, "workflowPresets",
    PresetKind::Workflow);
  m_PresetNamesByFile[filePath] = std::move(publishedNames);

  m_LoadedFiles.insert(filePath);
  return true;
}

void
PresetsGraph::RefreshInheritanceGraph()
{
  m_InheritanceGraph = PresetInheritanceGraph{};
  for(const auto* preset : m_PresetModel.GetPresets()) {
    if(preset->GetType() == PresetKind::Workflow) {
      continue;
    }

    auto inheritancePayload = PresetPayload{
      .Name = preset->GetName(),
      .Hidden = preset->GetHidden(),
      .UsesVendorMacro = HasVendorMacro(preset->GetRawJson()),
      .ConditionAst = nullptr,
      .PendingInherits = preset->GetInherits(),
      .Availability = PresetAvailability::Active,
      .IsUnresolved = false,
      .Reason = std::nullopt,
    };

    if(preset->GetCondition() != nullptr) {
      inheritancePayload.ConditionAst = preset->GetCondition()->Clone();
    }

    if(preset->GetIsUnresolved() && preset->GetReason().has_value()) {
      inheritancePayload.IsUnresolved = true;
      inheritancePayload.Reason = preset->GetReason();
    }

    m_InheritanceGraph.AddPreset(std::move(inheritancePayload));
  }
}

void
PresetsGraph::ValidateWorkflowPresets()
{
  for(const auto* preset : m_PresetModel.GetPresets()) {
    const auto* workflow = dynamic_cast<const WorkflowPreset*>(preset);
    if(workflow == nullptr) {
      continue;
    }

    const auto& steps = workflow->GetSteps();
    if(steps.empty() || steps.front().Type != WorkflowStepType::Configure
      || m_PresetModel.GetPreset<ConfigurePreset>(steps.front().Name)
        == nullptr)
    {
      m_WorkflowDiagnostics.push_back(WorkflowDiagnostic{
        .PresetName = workflow->GetName(),
        .Message = "First workflow step must reference a configure preset.",
      });
      continue;
    }

    const auto& configureName = steps.front().Name;
    for(size_t index = 1; index < steps.size(); ++index) {
      const ConfigureConsumerPreset* consumer = nullptr;
      if(steps[index].Type == WorkflowStepType::Build) {
        consumer = m_PresetModel.GetPreset<BuildPreset>(steps[index].Name);
      } else if(steps[index].Type == WorkflowStepType::Test) {
        consumer = m_PresetModel.GetPreset<TestPreset>(steps[index].Name);
      } else if(steps[index].Type == WorkflowStepType::Package) {
        consumer = m_PresetModel.GetPreset<PackagePreset>(steps[index].Name);
      }

      if(consumer == nullptr || consumer->GetConfigurePreset() != configureName)
      {
        m_WorkflowDiagnostics.push_back(WorkflowDiagnostic{
          .PresetName = workflow->GetName(),
          .Message =
            "Workflow step configurePreset does not match the first step.",
        });
      }
    }
  }
}

}  // namespace nhc::preset_graph
