## Context

The current workflow validation contract splits failure reporting across two surfaces: the workflow preset remains present in `PresetModel`, while `PresetsGraph` owns a separate list of workflow diagnostics. That preserves non-fatal validation, but it leaves callers without a single preset-level signal that a workflow is structurally invalid.

The architecture review explicitly called out this split and suggested surfacing workflow diagnostics on the preset itself. This change must preserve two existing constraints from the current model:
- workflow presets remain outside inheritance-graph participation
- workflow validation failures remain non-fatal to the manager refresh loop

The dependent `tighten-workflow-preset-api` change also keeps workflow presets on a narrow typed surface, so this change must not reintroduce inheritance-oriented or condition-oriented workflow behavior through the back door.

## Goals / Non-Goals

**Goals:**
- Give consumers a preset-level unresolved signal for invalid workflow structure.
- Preserve step-level workflow diagnostics so callers can inspect the exact validation failures.
- Keep workflow presets queryable in `PresetModel` even when validation fails.
- Keep workflow presets out of inheritance resolution and inheritance-graph nodes.

**Non-Goals:**
- Changing workflow-step parsing or the accepted step-shape rules
- Introducing workflow inheritance, condition, or environment semantics
- Replacing detailed workflow diagnostics with a single coarse unresolved flag
- Changing unrelated preset unresolved-state behavior

## Decisions

1. Mark invalid workflow presets unresolved in addition to recording workflow diagnostics.
   - Rationale: callers need one primary object-level state check for "is this workflow usable?" while still retaining detailed step diagnostics.
   - Alternative considered: keep diagnostics manager-only and require consumers to merge two sources. Rejected because it preserves the current contract gap.

2. Use a dedicated unresolved reason for workflow validation failure.
   - Rationale: a workflow-step mismatch is materially different from parse failures such as `InvalidCondition` or resolution failures such as `EnvironmentCycle`. A dedicated reason keeps preset-level state specific and queryable.
   - Alternative considered: reuse a generic unresolved reason or leave the reason unspecified. Rejected because it weakens downstream diagnostics and facade summaries.

3. Keep workflow diagnostics additive rather than replacing them with preset-level unresolved state.
   - Rationale: unresolved state answers whether the workflow is structurally valid; diagnostics answer why. Both are needed.
   - Alternative considered: collapse all workflow failures into the unresolved reason alone. Rejected because it loses per-step detail and forces string parsing or future API expansion.

4. Keep workflow validation outside inheritance resolution even when preset-level unresolved state is added.
   - Rationale: CMake workflow presets do not participate in inheritance, and the existing architecture already isolates them from inheritance nodes. This change is about surfacing, not eligibility.
   - Alternative considered: use inheritance-graph unresolved state as the transport for workflow failures. Rejected because it would blur subsystem ownership and violate the documented workflow boundary.

## Risks / Trade-offs

- [Risk] Existing consumers may currently treat workflow diagnostics as informational only and ignore preset unresolved state for workflows. → Mitigation: define the new behavior explicitly in both model and manager specs so the contract change is reviewable and testable.
- [Risk] The unresolved reason could be interpreted as replacing detailed diagnostics. → Mitigation: require diagnostics to remain attached to the workflow preset in parallel with the unresolved reason.
- [Risk] Future query surfaces may duplicate workflow state if they separately expose unresolved reasons and diagnostics. → Mitigation: keep the contract clear that unresolved reason is the summary signal and diagnostics are the detailed explanation.
