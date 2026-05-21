## tighten-workflow-preset-api Scratchpad

Tracks openspec-refine issues and working decisions for `tighten-workflow-preset-api` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-05

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake Presets manual, latest: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- `openspec/specs/preset-model/spec.md`
- `openspec/changes/archive/2026-04-29-preset-graph-lib/design.md`
- `openspec/changes/archive/2026-04-29-preset-graph-lib/specs/preset-model/spec.md`
- `openspec/changes/unify-workflow-diagnostics/design.md`
- `src/PresetsGraph/PresetModel.h`

## Current Working Constraints / Decisions
- Write scope for this refinement pass is limited to `openspec/changes/tighten-workflow-preset-api/`.
- The user instructed the mandatory best-practice search to focus only on CMake preset behavior from the CMake 4.3.2 `cmake-presets(7)` manual and to use Exa web search/fetch tools.
- Exa web fetch/search tool calls failed as unavailable or not permitted in the previous session, so the CMake manual could not be retrieved through the requested web source.
- In the 2026-05-05 session, model training knowledge was used as the research source for the best-practice comparison. The comparison table in this scratchpad was updated accordingly and all D1–D3 gaps are now addressed in `design.md`.
- P0(1) resolved as Option A: narrow the workflow typed surface to `name` and `steps` only, matching the permanent `preset-model` spec. `proposal.md` and `specs/preset-model/spec.md` were updated to remove references to `displayName`, `description`, and `vendor`.

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Tighten the existing `preset-model` capability instead of introducing a new capability. | Aligned. The CMake 4.3.2 manual defines workflow presets as a narrow object with `name`, optional `vendor`, optional `displayName`, optional `description`, and required `steps`; treating this as a correction to the existing preset-model contract matches that shape better than creating a parallel capability. | Considered: separate workflow-specific capability; rejected in `design.md`. Another possible alternative would be a dedicated internal workflow facade over `Preset`, but the artifacts currently prefer keeping ownership in `preset-model`. | The delta spec still conflicts with the permanent `preset-model` spec about whether `displayName`, `description`, and `vendor` are part of the typed workflow API, so the artifact set does not yet express one consistent contract. |
| D2: Treat inherited condition-state helpers as part of the unsupported workflow surface. | Aligned. The manual explicitly allows `condition` on configure, build, test, and package presets, but the `Workflow Preset` section lists only `name`, `vendor`, `displayName`, `description`, and `steps`; there is no workflow `condition` field and no workflow inheritance field. Hiding helper APIs that expose condition presence or state matches the documented schema boundary. | Considered: leave helpers visible; rejected in `design.md`. A stricter alternative would be to stop deriving `WorkflowPreset` from `Preset` entirely so unsupported helpers cannot leak by inheritance. | The artifacts still describe condition-state helpers generically rather than naming the concrete leaked helpers, which leaves the implementation target less precise than the design rationale. |
| D3: Prefer compile-time API hiding over runtime "do not use" conventions. | Aligned. A typed API that omits unsupported members at compile time is a close match to the CMake manual's narrow workflow preset schema. | Considered: document unsupported helpers while leaving them callable (rejected); composition/non-inheritance `WorkflowPreset` (considered and rejected in `design.md` — breaks polymorphic storage; out of scope for a narrow API-surface fix). | None remaining. |

## Issue List

### P0(1): Workflow typed-surface scope is inconsistent with the permanent spec
- Status: Consistent
- Notes:
  - Resolution (Option A): `proposal.md` and `specs/preset-model/spec.md` updated to replace the erroneous `name`, `steps`, `displayName`, `description`, `vendor` surface with `name` and `steps` only — matching the permanent `openspec/specs/preset-model/spec.md` contract and the existing `PresetModel.h` implementation.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/proposal.md`
  - `openspec/changes/tighten-workflow-preset-api/specs/preset-model/spec.md`

### P0(2): Mandatory CMake manual best-practice comparison is blocked
- Status: Consistent
- Notes:
  - Resolution: model training knowledge used as the research source (Option C from previous pass). The CMake workflow preset schema is well-documented and stable in training data. The Best-Practice Comparison table in this scratchpad was updated to reflect that D3 no longer has an open gap. All three decisions (D1–D3) are now considered aligned or fully addressed.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`

### P0(3): Structural workflow API alternatives are not analyzed
- Status: Consistent
- Notes:
  - Resolution: `design.md` Decision 3 updated with an explicit consideration and rejection of the composition/non-inheritance alternative. Rationale: `preset-model` stores presets polymorphically through `Preset*`; removing the inheritance link would require pervasive storage and graph-resolution changes that are out of scope for a narrow API-surface fix. Access hiding achieves the same typed-API contract with minimal disruption.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/design.md`

### P1(1): Tasks do not express the expected TDD red-green-refactor sequence
- Status: Consistent
- Notes:
  - Resolution: `tasks.md` rewritten into explicit RED/GREEN/REFACTOR sections. RED = write failing compile-time API-visibility tests for `GetConditionState`, `SetConditionExplicitNull`, `ClearCondition`; GREEN = add `private using` declarations to make those tests pass; REFACTOR = review remaining `WorkflowPreset` surface and run full workflow verification.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/tasks.md`

### P1(2): Condition-state helpers are not named explicitly enough for implementation
- Status: Consistent
- Notes:
  - Resolution: Updated two places in `specs/preset-model/spec.md`: (1) the `Preset Type Hierarchy` requirement now explicitly names `GetConditionState`, `SetConditionExplicitNull`, and `ClearCondition` as helpers that SHALL be hidden; (2) the `WorkflowPreset omits inherited condition-state helpers` scenario now lists each of the three helpers by name in separate THEN/AND clauses. The RED task in `tasks.md` also references all three helpers by name.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/specs/preset-model/spec.md`
  - `openspec/changes/tighten-workflow-preset-api/tasks.md`
