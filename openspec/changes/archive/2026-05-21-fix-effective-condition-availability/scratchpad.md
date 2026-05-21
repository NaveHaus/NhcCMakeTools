## fix-effective-condition-availability Scratchpad

Tracks openspec-refine issues and working decisions for `fix-effective-condition-availability` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-05

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake 4.3.2 `cmake-presets(7)` manual requested by the user: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- Retrieval status: Successfully fetched via Exa in session 2026-05-05. Manual content confirmed all three design decisions (D1, D2, D3) and verified that workflow presets carry no `condition` field.
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
- Best-practice comparison is complete. CMake 4.3.2 manual was fetched via Exa in session 2026-05-05 and verified all three design decisions.

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|--------------|------|
| D1: Use `PresetModel::ResolveCondition()` as the single effective-condition source | Aligned. The CMake 4.3.2 manual defines `condition` as a preset property that participates in normal `inherits` precedence, with `null` enabled locally and not inherited by descendants. Centralizing that resolution in the model matches the manual's per-preset condition semantics and avoids a second inheritance implementation in the graph layer. | Considered: reimplement effective-condition lookup in graph refresh. Rejected in `design.md`; no additional manual-backed alternative is better than a single resolver for inherited `condition` semantics. | The current artifacts still describe the inheritance-graph side more clearly than the manager publication contract, so the source-of-truth boundary is sound but not fully propagated across all related specs. |
| D2: Add cycle protection inside effective-condition resolution | Room for improvement. The manual explicitly forbids include cycles among preset files and constrains environment self-reference cycles, so adding defensive cycle handling for `inherits` resolution is directionally consistent with CMake's broader preset validation model. However, the manual excerpt retrieved through Exa does not state an `inherits` cycle rule for preset-to-preset condition traversal, so this remains a repository design choice rather than a directly specified CMake behavior. | Considered: detect cycles before payload publishing; throw or return a separate cycle error. Those alternatives are documented in `design.md`, satisfying the requirement to consider alternatives. | The artifacts should be explicit that cycle-safe resolution is an internal safety contract for refresh-time lookup, while authoritative cycle diagnostics still belong to the inheritance graph because the manual evidence here is analogous, not direct. |
| D3: Publish effective conditions into inheritance payloads | Aligned. The manual defines `condition` as the enablement gate for configure, build, test, and package presets. Workflow presets carry no `condition` field at all (confirmed by CMake 4.3.2 manual — the Workflow Preset section lists only `name`, `vendor`, `displayName`, `description`, and `steps`). Precomputing the effective inherited condition into inheritance payloads is consistent with per-preset condition semantics across those preset kinds while correctly excluding workflow presets. | Considered: leave payloads direct-only and climb parents dynamically during availability evaluation. Rejected in `design.md`; it would duplicate `inherits` precedence outside the model and drift from the manual's single resolved-preset view. | Gap resolved. A new `preset-graph-manager` delta spec (P1(1)) now states that Configure, Build, Test, and Package presets are published via `ResolveCondition()` while workflow presets are explicitly excluded. |

Manual points used in this comparison from CMake 4.3.2 `cmake-presets(7)` fetched via Exa:
1. `condition` may be `boolean`, `null`, or an object; `null` means the preset is enabled and that `null` condition is not inherited by descendants.
2. Configure, build, test, and package presets each support `inherits` and `condition`; workflow presets instead define ordered `steps` that reference those presets.
3. CMake documents cycle rejection in related preset mechanisms such as file `include` relationships and environment-variable self-reference, supporting the design choice to make condition lookup cycle-safe even though the manual text fetched here does not define `inherits` cycle handling explicitly.

## Issue List

### P1(1): Manager refresh contract omits effective-condition payload publication
- Status: Consistent
- Notes:
  - Added `openspec/changes/fix-effective-condition-availability/specs/preset-graph-manager/spec.md` with MODIFIED requirement "Effective-Condition Payload Publishing". Defines that the Manager calls `PresetModel::ResolveCondition()` for each Configure, Build, Test, and Package preset and stores the result in the Inheritance Graph payload before availability evaluation. Workflow presets are explicitly excluded (CMake 4.3.2 manual confirms they carry no `condition` field).
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/specs/preset-graph-manager/spec.md` (created)

### P1(2): Effective explicit-null representation is ambiguous across artifacts
- Status: Consistent
- Notes:
  - Resolved in favor of the model spec's "absent" representation. `ResolveCondition()` returns absent when traversal terminates at explicit `null`. The inheritance-graph delta "Condition Status Tracking" availability rule now says "absent" only (removed "or explicit `null`"). The "Explicit null condition is active" scenario was rewritten as "Preset with null condition has absent effective condition and is active" using GIVEN/WHEN/THEN to reflect the model-layer contract.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/specs/preset-inheritance-graph/spec.md`

### P1(3): Mandatory CMake preset best-practice comparison is blocked
- Status: Consistent
- Notes:
  - Exa fetch succeeded in session 2026-05-05. CMake 4.3.2 `cmake-presets(7)` manual was retrieved and reviewed. All three design decisions (D1, D2, D3) are confirmed or appropriately noted as repo design choices. Best-Practice Comparison table updated with D3 workflow-preset exclusion confirmation. Retrieval status and working constraints updated.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/scratchpad.md`

### P1(4): Cycle diagnostic ownership is not regression-tested
- Status: Consistent
- Notes:
  - Added scenario "Cycle diagnostics are preserved after effective-condition publishing" to `specs/preset-inheritance-graph/spec.md` delta. Scenario asserts that after effective-condition publishing, the Inheritance Graph still reports `A` and `B` as Unresolved with reason `InheritanceCycle` and that cycle-safe lookup does not suppress or replace that diagnostic.
  - Added task 3.4 to `tasks.md` requiring a REGRESSION test exercising a cyclic inherits graph after effective-condition publishing.
- Artifacts touched:
  - `openspec/changes/fix-effective-condition-availability/specs/preset-inheritance-graph/spec.md`
  - `openspec/changes/fix-effective-condition-availability/tasks.md`
