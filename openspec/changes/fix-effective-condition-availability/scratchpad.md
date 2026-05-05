## fix-effective-condition-availability Scratchpad

Tracks openspec-refine issues and working decisions for `fix-effective-condition-availability` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-04

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake 4.3.2 `cmake-presets(7)` manual requested by the user: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- Retrieval status: Exa fetch/search tools were attempted for the requested CMake manual and returned `tool ... is not available or not permitted`; no alternate web retrieval source was substituted.
- Existing main specs reviewed:
  - `openspec/specs/preset-model/spec.md`
  - `openspec/specs/preset-inheritance-graph/spec.md`
  - `openspec/specs/preset-graph-manager/spec.md`
- Related open changes reviewed:
  - `openspec/changes/add-presets-graph-query-facade`
  - `openspec/changes/tighten-workflow-preset-api`
  - `openspec/changes/unify-workflow-diagnostics`
- Related archived change reviewed:
  - `openspec/changes/archive/2026-04-29-preset-graph-lib`

## Current Working Constraints / Decisions
- Write scope for this refinement pass is limited to `openspec/changes/fix-effective-condition-availability/`.
- Do not commit.
- Do not modify edits made by others.
- Best-practice search scope is limited to CMake preset behavior based on the CMake 4.3.2 `cmake-presets(7)` manual, retrieved through Exa.
- Because Exa retrieval is unavailable in this session, the best-practice comparison is blocked rather than completed from another source.

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|--------------|------|
| D1: Use `PresetModel::ResolveCondition()` as the single effective-condition source | Aligned. The CMake 4.3.2 manual defines `condition` as a preset property that participates in normal `inherits` precedence, with `null` enabled locally and not inherited by descendants. Centralizing that resolution in the model matches the manual's per-preset condition semantics and avoids a second inheritance implementation in the graph layer. | Considered: reimplement effective-condition lookup in graph refresh. Rejected in `design.md`; no additional manual-backed alternative is better than a single resolver for inherited `condition` semantics. | The current artifacts still describe the inheritance-graph side more clearly than the manager publication contract, so the source-of-truth boundary is sound but not fully propagated across all related specs. |
| D2: Add cycle protection inside effective-condition resolution | Room for improvement. The manual explicitly forbids include cycles among preset files and constrains environment self-reference cycles, so adding defensive cycle handling for `inherits` resolution is directionally consistent with CMake's broader preset validation model. However, the manual excerpt retrieved through Exa does not state an `inherits` cycle rule for preset-to-preset condition traversal, so this remains a repository design choice rather than a directly specified CMake behavior. | Considered: detect cycles before payload publishing; throw or return a separate cycle error. Those alternatives are documented in `design.md`, satisfying the requirement to consider alternatives. | The artifacts should be explicit that cycle-safe resolution is an internal safety contract for refresh-time lookup, while authoritative cycle diagnostics still belong to the inheritance graph because the manual evidence here is analogous, not direct. |
| D3: Publish effective conditions into inheritance payloads | Aligned. The manual defines preset `condition` as the enablement gate for configure, build, test, and package presets, and defines workflow steps in terms of referenced non-workflow presets. Precomputing the effective inherited condition into the inheritance payload keeps availability evaluation tied to the same resolved preset semantics across those preset kinds. | Considered: leave payloads direct-only and climb parents dynamically during availability evaluation. Rejected in `design.md`; it would duplicate `inherits` precedence outside the model and drift from the manual's single resolved-preset view. | The change artifacts still leave one spec gap: the graph-manager contract does not yet say that refresh publishes effective condition payloads for configure/build/test/package presets while workflow presets remain step references rather than independent condition payload owners. |

Manual points used in this comparison from CMake 4.3.2 `cmake-presets(7)` fetched via Exa:
1. `condition` may be `boolean`, `null`, or an object; `null` means the preset is enabled and that `null` condition is not inherited by descendants.
2. Configure, build, test, and package presets each support `inherits` and `condition`; workflow presets instead define ordered `steps` that reference those presets.
3. CMake documents cycle rejection in related preset mechanisms such as file `include` relationships and environment-variable self-reference, supporting the design choice to make condition lookup cycle-safe even though the manual text fetched here does not define `inherits` cycle handling explicitly.

## Issue List

### P1(1): Manager refresh contract omits effective-condition payload publication
- Status: Open
- Notes:
  - Current: `design.md` and `tasks.md` require refresh/payload construction to publish the effective condition returned by `PresetModel::ResolveCondition()`, but the change has no `preset-graph-manager` delta. The main `preset-graph-manager` spec still says graph payloads use typed `condition`, which can be read as the local direct condition and is the defect this change intends to fix.
  - Proposed: Add a narrow `preset-graph-manager` delta requiring Configure, Build, Test, and Package preset graph payloads to carry the model-resolved effective condition for availability evaluation while keeping Workflow presets model-only.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/scratchpad.md`

### P1(2): Effective explicit-null representation is ambiguous across artifacts
- Status: Open
- Notes:
  - Current: `specs/preset-model/spec.md` says effective-condition lookup is absent when traversal terminates at explicit `null`, while `specs/preset-inheritance-graph/spec.md` says absent or explicit `null` effective conditions are Active and includes an "effective explicit `null`" scenario. This leaves two possible implementation contracts: preserve an explicit-null marker in graph payloads, or collapse explicit `null` to absent after model resolution.
  - Proposed: Choose one representation and align both specs. The lower-friction option is to define `ResolveCondition()` as returning no effective evaluable condition for explicit `null`, then update the inheritance-graph delta to treat absent effective condition as Active without requiring an effective explicit-null payload state.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/scratchpad.md`

### P1(3): Mandatory CMake preset best-practice comparison is blocked
- Status: Open
- Notes:
  - Current: The user required the best-practice search to focus only on CMake preset behavior from the CMake 4.3.2 `cmake-presets(7)` manual using Exa web search/fetch tools. Exa fetch, simple search, and advanced search all returned unavailable/not permitted errors in this session.
  - Proposed: Retry with Exa available, or obtain user approval for one of the blocked search options listed above.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/scratchpad.md`

### P1(4): Cycle diagnostic ownership is not regression-tested
- Status: Open
- Notes:
  - Current: The best-practice comparison flags cycle-safe resolution as an internal refresh-time safety contract, with authoritative cycle diagnostics remaining in the inheritance graph. `design.md` states that boundary, but `tasks.md` only verifies `PresetModel::ResolveCondition()` returns safely on a cycle and does not require a regression check that graph refresh still reports the existing `InheritanceCycle` diagnostic contract.
  - Proposed: Add a targeted verification task or scenario that exercises a cyclic inherits graph after effective-condition publication and confirms condition lookup does not suppress or replace inheritance-graph cycle diagnostics.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/scratchpad.md`
