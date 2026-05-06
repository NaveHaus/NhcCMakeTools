## fix-include-resolution-policy Scratchpad

Tracks openspec-refine issues and working decisions for `fix-include-resolution-policy` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-05

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- Requested best-practice source: CMake 4.3.2 `cmake-presets(7)` manual at <https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html>.
- Local spec reference: `openspec/specs/preset-include-graph/spec.md`.
- Local spec reference: `openspec/specs/preset-graph-manager/spec.md`.
- Local active change reference: `openspec/changes/fix-manager-version-and-reapply-state/`.
- Local active change reference: `openspec/changes/add-presets-graph-query-facade/`.

## Current Working Constraints / Decisions
- Write scope is limited to `openspec/changes/fix-include-resolution-policy/`.
- Do not commit.
- Best-practice search must use Exa web search/fetch tools and focus only on CMake preset behavior from the CMake 4.3.2 `cmake-presets(7)` manual.
- Exa `fetch`, `search`, and `advanced search` tool calls returned "not available or not permitted" during this refinement pass.
- No substitute source was used for best-practice conclusions because the user constrained the source and retrieval method.

## Best-Practice Comparison

Source: CMake 4.3.2 `cmake-presets(7)` manual — retrieved 2026-05-05 via Exa fetch from https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html.

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Delegate include expansion policy to `PresetIncludeGraph` | **Confirmed aligned** (CMake 4.3.2 manual). Manual defines: (1) include paths are relative to the current file; (2) include cycles are forbidden; (3) v7–8 allows only `$penv{}` in `include`; (4) v9+ allows other macros except `$env{}` and preset-specific macros; (5) a file may be included multiple times from the same file or from different files. Keeping that policy in one resolver matches the manual's per-file rule set and avoids duplicating a multi-rule contract in the manager. | Considered: re-implement in `PresetsGraph::ApplyContext()`; move discovered-file loading into `PresetIncludeGraph`. Both rejected. | ~~Relative-path and cycle constraints not explicitly captured~~ — confirmed covered: relative-path is in the permanent include-graph spec; cycle rejection is in the permanent manager spec's Resolution Cycle Detection requirement. Repeated inclusion tolerance (manual: explicitly allowed) was missing — added as a scenario to the delta spec (P1(5) remediation). |
| D2: Keep manager orchestration focused on discovery and refresh | **Confirmed aligned**. Manual: "Files included by these files can also include other files" — transitive inclusion is a CMake-defined requirement, directly matched by the manager's iterative discovery loop. Cross-file inheritance validity is a downstream consequence of correct include resolution, not a separate manager contract. | Considered: collapse include resolution and loading into a single component. Rejected. | ~~Iterative loop not tied to transitive model~~ — confirmed resolved: the iterative loop is the direct implementation of the manual's transitive include requirement. No spec change needed. |
| D3: Test the integration at the manager boundary | **Confirmed aligned**. Manual version-gate rules for `$penv{}` (v7–8) and the v9+ macro expansion are the exact basis for the RED tests. Correctness can fail at the integration boundary even when the include resolver is correct in isolation. | Considered: rely only on existing include-graph tests. Rejected. | ~~Relative-path, repeated-inclusion, include-cycle tests not explicitly called out~~ — confirmed: (1) relative-path resolution is internal to `PresetIncludeGraph`, tested in `GraphIncludeTests`, and covered by the permanent include-graph spec; (2) include-cycle detection is in the manager's iterative loop, not in the delegation path, and covered by the permanent manager spec's Resolution Cycle Detection requirement; (3) repeated-inclusion scenario added to delta spec; task 1.7 requires a manager-boundary test if no existing coverage is found. |

## Issue List

### P1(1): Best-practice search source unavailable
- Status: Consistent
- Notes:
  - Exa fetch succeeded on 2026-05-05 and retrieved the full CMake 4.3.2 `cmake-presets(7)` manual.
  - All three design decisions (D1–D3) are confirmed aligned with the manual.
  - The previously noted gaps (relative-path constraint, include-cycle constraint) are confirmed covered by existing permanent specs.
  - One new gap was found: repeated inclusion tolerance ("a file may be included multiple times from the same file or from different files") was not captured anywhere — addressed by P1(5) remediation.
  - Best-Practice Comparison table updated with confirmed findings.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/scratchpad.md`

### P1(2): Artifact quality and implementation readiness
- Status: Consistent
- Notes:
  - `spec.md` contains normative behavior only and each requirement has measurable scenarios.
  - `tasks.md` is broken into RED/GREEN/REFACTOR implementation tasks with clear affected test target and verification steps.
  - Design decisions include rationale and rejected alternatives.
  - Error states for unsupported include macros are captured through the existing `UnsupportedMacro` diagnostic contract.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/proposal.md`
  - `openspec/changes/fix-include-resolution-policy/design.md`
  - `openspec/changes/fix-include-resolution-policy/tasks.md`
  - `openspec/changes/fix-include-resolution-policy/specs/preset-graph-manager/spec.md`

### P1(3): Cross-artifact consistency
- Status: Consistent
- Notes:
  - Proposal, design, tasks, and delta spec consistently limit this change to manager delegation of include macro-policy enforcement.
  - Design non-goals do not contradict proposal capabilities.
  - Normative behavior in design is represented in `specs/preset-graph-manager/spec.md`.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/proposal.md`
  - `openspec/changes/fix-include-resolution-policy/design.md`
  - `openspec/changes/fix-include-resolution-policy/tasks.md`
  - `openspec/changes/fix-include-resolution-policy/specs/preset-graph-manager/spec.md`

### P1(4): Commonality with existing specs and active changes
- Status: Consistent
- Notes:
  - Reuse decision: continue using `PresetIncludeGraph` as the authoritative include macro-policy component.
  - Extend decision: extend `preset-graph-manager` behavior so it delegates to, and preserves diagnostics from, the include graph.
  - No active change found that attempts to move include macro policy out of `PresetIncludeGraph`.
  - Related active changes are compatible: `fix-manager-version-and-reapply-state` explicitly avoids refactoring include resolution ownership, and `add-presets-graph-query-facade` depends on lower-layer manager/include graph state without redefining it.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/proposal.md`
  - `openspec/changes/fix-include-resolution-policy/design.md`
  - `openspec/changes/fix-include-resolution-policy/tasks.md`
  - `openspec/changes/fix-include-resolution-policy/specs/preset-graph-manager/spec.md`

### P1(5): Manager delegation tests omit non-macro include semantics
- Status: Consistent
- Notes:
  - CMake 4.3.2 manual confirms three non-macro include semantics: relative-path resolution, repeated inclusion tolerance, include-cycle rejection.
  - Relative-path resolution: internal to `PresetIncludeGraph`, covered by the permanent include-graph spec ("interpreted relative to the directory of the including file node") and existing `GraphIncludeTests`. Not affected by the delegation refactor.
  - Include-cycle rejection: implemented in the manager's iterative loop, not in the delegation path. Covered by the permanent manager spec's Resolution Cycle Detection requirement. Not affected by the delegation refactor.
  - Repeated inclusion tolerance: CMake manual explicitly states "a file may be included multiple times from the same file or from different files." This was not captured in any spec — added as a new scenario ("Applying context tolerates repeated inclusion of the same file") to the "Context Application Loop" MODIFIED requirement in the delta spec.
  - Task 1.7 added to the REFACTOR section to require a manager-boundary test for repeated inclusion if no existing coverage is found during implementation.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/tasks.md`
  - `openspec/changes/fix-include-resolution-policy/specs/preset-graph-manager/spec.md`

### P2(1): Design architecture checks
- Status: Consistent
- Notes:
  - Separation of concerns is explicit: include macro validation remains in `PresetIncludeGraph`; file loading and iterative refresh remain in `PresetsGraph`.
  - Dependencies remain acyclic at the artifact level because the manager consumes include-graph results rather than moving manager loading responsibilities into the include graph.
  - Rejected patterns are documented for duplicating include policy, moving loading into the include graph, collapsing components, and relying only on include-graph tests.
- Artifacts touched:
  - `openspec/changes/fix-include-resolution-policy/design.md`
