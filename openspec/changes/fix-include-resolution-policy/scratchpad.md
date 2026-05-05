## fix-include-resolution-policy Scratchpad

Tracks openspec-refine issues and working decisions for `fix-include-resolution-policy` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-04

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

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Delegate include expansion policy to `PresetIncludeGraph` | Aligned. The CMake 4.3.2 manual defines include expansion as a file-level rule set with version-gated macro support: include paths are relative to the current file, include cycles are forbidden, version 7 permits only `$penv{}` in `include`, and version 9 adds other macros except `$env{}` and preset-specific macros. Keeping that policy in one resolver matches the spec better than duplicating it in the manager. | Considered: re-implement include policy in `PresetsGraph::ApplyContext()`; move discovered-file loading into `PresetIncludeGraph`. The first alternative conflicts with the manual's single include-behavior contract by creating a second policy implementation. The second would mix policy with file-loading orchestration not implied by the manual. | The artifacts compare against macro-allowance behavior, but they do not explicitly mention other include constraints from the manual such as "relative to the current file" and "include cycles are not allowed." |
| D2: Keep manager orchestration focused on discovery and refresh | Aligned with room for clarification. The manual describes include processing as transitive across files and requires that inheritance across files is valid only when the including chain exists directly or indirectly. That supports a manager loop which resolves includes, loads newly discovered files, and refreshes state until no new files appear, while leaving per-include semantics to the include graph. | Considered: collapse include resolution and loading into a single component. The manual does not require that structure; it only requires correct transitive inclusion behavior, so keeping orchestration separate remains a reasonable design choice. | The artifacts do not explicitly tie the iterative refresh loop to the manual's transitive include model or to the requirement that cross-file inheritance is valid only through an established include chain. |
| D3: Test the integration at the manager boundary | Aligned. The manual's include behavior depends on preset-file version and macro class, so correctness can fail at the integration boundary even when the include resolver works in isolation. Manager-level tests for `$env{}`, `${presetName}`, and version-gated `${fileDir}` directly exercise the observable behavior CMake specifies for `include`. | Considered: rely only on existing include-graph tests. That misses the documented version-sensitive behavior once the manager constructs per-file macro context and invokes include resolution. | The artifacts focus on unsupported macro diagnostics and do not explicitly call out tests for other documented include semantics such as relative-path resolution, repeated inclusion tolerance, or include-cycle rejection. |

## Issue List

### P1(1): Best-practice search source unavailable
- Status: Open
- Notes:
  - The mandatory best-practice search could not be completed because all Exa web retrieval attempts failed with "not available or not permitted."
  - The requested source and retrieval constraint prevents replacing this step with another source.
  - Options for a future pass:
    - Enable Exa access and rerun the best-practice comparison against the CMake 4.3.2 `cmake-presets(7)` manual.
    - Permit a different retrieval method for the same CMake manual URL.
    - Explicitly accept the local-only refinement result without external best-practice confirmation.
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
- Status: Open
- Notes:
  - The best-practice comparison identifies relative-path resolution, repeated inclusion tolerance, and include-cycle rejection as documented include semantics that can be affected by changing the manager/include-graph delegation boundary.
  - Existing permanent specs already define relative include handling and include-cycle rejection, but this change's implementation tasks only require manager-boundary tests for unsupported macro diagnostics.
  - Add targeted manager regression coverage for the existing non-macro include semantics affected by `ApplyContext()` delegation, or explicitly document why existing coverage is sufficient for this refactor.
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
