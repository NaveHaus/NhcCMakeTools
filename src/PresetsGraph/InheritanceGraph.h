/// @file InheritanceGraph.h
/// @brief Defines preset inheritance graph behavior.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Condition.h"
#include "Graph.h"
#include "IncludeGraph.h"
#include "MacroContext.h"

namespace nhc::preset_graph {

enum class InheritanceGraphState
{
  Empty,
  Resolved,
  Unresolved
};

enum class PresetAvailability
{
  Active,
  Hidden,
  Disabled,
  Unknown
};

/// Payload stored for a preset node.
struct PresetPayload
{
  std::string Name;
  bool Hidden = false;
  bool UsesVendorMacro = false;
  std::unique_ptr<Condition> ConditionAst;
  std::vector<std::string> PendingInherits;

  PresetAvailability Availability = PresetAvailability::Active;
  bool IsUnresolved = false;
  std::optional<UnresolvedReason> Reason;
};

/// Graph model for preset inheritance relationships.
class PresetInheritanceGraph
{
  public:
  using NodeId = Graph<PresetPayload>::NodeId;

  NodeId AddPreset(PresetPayload payload);

  const PresetPayload& GetPresetPayload(NodeId nodeId) const;

  const std::vector<NodeId>& GetInheritedPresets(NodeId nodeId) const;

  void Resolve(const MacroContext& context);

  InheritanceGraphState ComputeState() const;

  private:
  void ResolveInheritanceLinks();
  void EvaluateAvailability(const MacroContext& context);
  void DetectCycles();

  void MarkUnresolved(NodeId nodeId, UnresolvedReason reason);

  Graph<PresetPayload> m_Graph;
};

}  // namespace nhc::preset_graph
