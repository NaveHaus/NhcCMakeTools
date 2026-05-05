## tighten-workflow-preset-api Scratchpad

Tracks openspec-refine issues and working decisions for `tighten-workflow-preset-api` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-04

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
- Exa web fetch/search tool calls failed as unavailable or not permitted in this session, so the CMake manual could not be retrieved through the requested web source.
- No changes were made to `proposal.md`, `design.md`, `tasks.md`, or `specs/preset-model/spec.md` in this pass because remediation requires choosing whether this change may broaden the workflow typed API beyond the currently permanent `preset-model` contract.

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Tighten the existing `preset-model` capability instead of introducing a new capability. | Aligned. The CMake 4.3.2 manual defines workflow presets as a narrow object with `name`, optional `vendor`, optional `displayName`, optional `description`, and required `steps`; treating this as a correction to the existing preset-model contract matches that shape better than creating a parallel capability. | Considered: separate workflow-specific capability; rejected in `design.md`. Another possible alternative would be a dedicated internal workflow facade over `Preset`, but the artifacts currently prefer keeping ownership in `preset-model`. | The delta spec still conflicts with the permanent `preset-model` spec about whether `displayName`, `description`, and `vendor` are part of the typed workflow API, so the artifact set does not yet express one consistent contract. |
| D2: Treat inherited condition-state helpers as part of the unsupported workflow surface. | Aligned. The manual explicitly allows `condition` on configure, build, test, and package presets, but the `Workflow Preset` section lists only `name`, `vendor`, `displayName`, `description`, and `steps`; there is no workflow `condition` field and no workflow inheritance field. Hiding helper APIs that expose condition presence or state matches the documented schema boundary. | Considered: leave helpers visible; rejected in `design.md`. A stricter alternative would be to stop deriving `WorkflowPreset` from `Preset` entirely so unsupported helpers cannot leak by inheritance. | The artifacts still describe condition-state helpers generically rather than naming the concrete leaked helpers, which leaves the implementation target less precise than the design rationale. |
| D3: Prefer compile-time API hiding over runtime "do not use" conventions. | Room for improvement, but directionally aligned. Because the manual models workflow presets as a distinct schema with a short fixed field list, a typed API that simply omits unsupported members is a closer match than leaving them callable and relying on runtime discipline. | Considered: document unsupported helpers while leaving them callable; rejected in `design.md`. Another alternative is composition instead of inheritance, which would enforce the same boundary structurally rather than with access hiding. | `design.md` rejects the runtime-only approach, but it does not discuss composition or a non-inheritance workflow type as an alternative, so the alternatives analysis is incomplete for a P0 best-practice check. |

## Issue List

### P0(1): Workflow typed-surface scope is inconsistent with the permanent spec
- Status: Open
- Notes:
  - `proposal.md` says this change preserves a workflow-facing typed surface containing `name`, `steps`, `displayName`, `description`, and `vendor`.
  - `specs/preset-model/spec.md` modifies the `Preset Type Hierarchy` requirement to say `WorkflowPreset` exposes `name`, `steps`, `displayName`, `description`, and `vendor`.
  - The permanent `openspec/specs/preset-model/spec.md` currently says `WorkflowPreset` exposes only `name` and `steps` in both `Preset Type Hierarchy` and `Library-Relevant Expanded Fields`.
  - `src/PresetsGraph/PresetModel.h` currently exposes `GetName()` through `Preset` and `GetSteps()` on `WorkflowPreset`; it does not expose typed `displayName`, `description`, or `vendor` accessors.
  - This is a P0 because the current artifacts can be implemented either as a narrow condition-helper hiding fix or as a broader workflow typed-surface expansion, and those are different API changes.
- Proposed remediation options:
  - Option A: Narrow this change to the existing permanent typed contract by changing the delta spec and proposal/design text back to `name` and `steps` only, while explicitly hiding inherited condition-state helpers.
  - Option B: Keep `displayName`, `description`, and `vendor` in scope, but update `proposal.md`, `design.md`, `tasks.md`, and scenarios to state this change intentionally broadens the workflow typed API and to add implementation/test tasks for those accessors.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`

### P0(2): Mandatory CMake manual best-practice comparison is blocked
- Status: Open
- Notes:
  - The user selected the CMake 4.3.2 `cmake-presets(7)` manual as the only best-practice source and required Exa web search/fetch tools for retrieval.
  - Exa fetch, search, and advanced search tool calls all failed as unavailable or not permitted.
  - The skill requires a web-backed comparison for each `design.md` decision; that comparison could not be completed from the requested source in this session.
- Proposed remediation options:
  - Option A: Re-run refinement when Exa web retrieval is available.
  - Option B: Authorize a non-Exa retrieval source for the CMake manual.
  - Option C: Authorize model training knowledge as the research source for this refinement pass.
  - Option D: Stop the search and leave the best-practice comparison unresolved.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`

### P0(3): Structural workflow API alternatives are not analyzed
- Status: Open
- Notes:
  - The best-practice comparison for D3 identifies compile-time API hiding as directionally aligned, but it also identifies composition or a non-inheritance `WorkflowPreset` type as an unaddressed alternative.
  - `design.md` rejects only a runtime documentation convention while leaving helpers callable; it does not explain why access hiding through inheritance is preferred over structurally preventing unsupported inherited members from existing on `WorkflowPreset`.
  - This is a P0 because the refinement rubric requires alternative approaches to be considered for each design decision, and this alternative affects the core shape of the public typed API.
- Proposed remediation:
  - Update `design.md` to either consider and reject composition/non-inheritance for `WorkflowPreset` with rationale, or adopt it and propagate the resulting scope changes through the spec and tasks.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`

### P1(1): Tasks do not express the expected TDD red-green-refactor sequence
- Status: Open
- Notes:
  - `tasks.md` identifies implementation and test work, but the test tasks are not written as explicit RED/GREEN/REFACTOR steps.
  - Repository instructions require the TDD skill for testing new or changed code and for generating testing tasks in OpenSpec `tasks.md`.
  - The current tasks are implementable, but making the compile-time API tests the RED step would improve implementation readiness and align the plan with repository process.
- Proposed remediation:
  - Rewrite the task list minimally so the first focused work item is a failing compile-time API-visibility test, the next work item hides the remaining inherited helpers, and the final work item refactors/verifies without changing behavior.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`

### P1(2): Condition-state helpers are not named explicitly enough for implementation
- Status: Open
- Notes:
  - `design.md` says the mitigation is to name condition-state helpers explicitly in the requirement and scenarios.
  - The delta spec currently says "helpers that expose workflow condition state" and "inherited helper APIs" but does not name `GetConditionState()`, `SetConditionExplicitNull()`, or `ClearCondition()`.
  - `src/PresetsGraph/PresetModel.h` currently hides `GetCondition()` and `SetCondition()` from `WorkflowPreset`, but not `GetConditionState()`, `SetConditionExplicitNull()`, or `ClearCondition()`.
- Proposed remediation:
  - Name the remaining condition-state helpers explicitly in the spec scenario and add a task/test that verifies all three are unavailable on `WorkflowPreset`.
- Artifacts touched:
  - `openspec/changes/tighten-workflow-preset-api/scratchpad.md`
