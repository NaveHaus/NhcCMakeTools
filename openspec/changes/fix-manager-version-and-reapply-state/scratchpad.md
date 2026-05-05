## fix-manager-version-and-reapply-state Scratchpad

Tracks openspec-refine issues and working decisions for `fix-manager-version-and-reapply-state` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-04

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake 4.3.2 `cmake-presets(7)` manual: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html

## Current Working Constraints / Decisions
- Write scope is limited to `openspec/changes/fix-manager-version-and-reapply-state/`.
- Do not commit this refinement.
- Do not modify or revert edits made by others.
- Mandatory best-practice search was narrowed by the user to CMake preset behavior in the CMake 4.3.2 `cmake-presets(7)` manual.
- Exa web search/fetch tools were required for retrieving web data, but all Exa calls failed with `tool ... is not available or not permitted`.
- No proposal, design, task, or spec remediation was applied because the only unresolved issue requires either retrying the unavailable research tool or choosing an alternate research source.

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|--------------|------|
| D1: Treat CMake 4.0 through 4.2 as version-10 managers | Matches the CMake 4.3.2 `cmake-presets(7)` version table. The manual says preset file version `10` was added in CMake `3.31`, and version `11` was added in CMake `4.3`. That makes the `design.md` boundary correct: simulated CMake `4.0` through `4.2` should still report version `10`, and `4.3` is the first release that should report `11`. | Considered in `design.md`: keep `major >= 4`; table-driven mapping. The `major >= 4` alternative conflicts with the manual because it would incorrectly grant version-11 support to CMake `4.0`, `4.1`, and `4.2`. A table-driven mapping would also align with the manual, but it is broader than this change requires. | The artifacts pin the `4.2` and `4.3` boundary explicitly, which is correct for current CMake behavior. The remaining gap is maintenance risk: future preset-file versions will require updating the hand-coded mapping again. |
| D2: Clear unresolved load state at the start of each reload attempt | Consistent with the observable behavior implied by the CMake 4.3.2 `cmake-presets(7)` manual. The manual defines preset loading from the current file set, says includes are resolved from the current file, disallows include cycles, and requires cross-file inheritance to come only from directly or indirectly included files. That behavior is based on the current reload attempt's inputs, not stale diagnostics from an earlier failed attempt. Clearing the target file node's unresolved state before retrying keeps manager state aligned with that model. | Considered in `design.md`: rebuild include graph on every apply; add bulk graph reset API. Both alternatives would also eliminate stale state, but they go beyond what the manual requires. The current design is the narrower fix because it recomputes the retried file's load state without changing unrelated graph identity or ownership. | The manual specifies observable preset semantics, not internal cache or node-lifecycle rules, so it cannot mandate this exact reset point. The artifact rationale is therefore inferential: this reset is justified because it preserves CMake-compatible reload results, not because the manual prescribes a specific internal implementation. |

## Issue List

### P1(1): CMake preset best-practice comparison blocked by unavailable Exa tools
- Status: Open
- Notes:
  - The user selected the CMake 4.3.2 `cmake-presets(7)` manual as the authoritative source and required Exa web search/fetch tools for retrieval.
  - Calls to `exa_web_search_advanced_exa`, `exa_web_search_exa`, and `exa_web_fetch_exa` failed with `tool ... is not available or not permitted`.
  - Artifact quality, cross-artifact consistency, commonality, and design checks found no P0 remediation required in the current artifacts.
  - Remediation options for a future pass:
    - Retry the Exa search/fetch step when the tool is available.
    - Approve an alternate official-doc retrieval path for the same CMake manual.
    - Explicitly accept the current artifacts without completing the external best-practice comparison.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`

### P1(2): Retry failure diagnostics after unresolved-state reset are not specified
- Status: Open
- Notes:
  - The best-practice comparison for D2 accepts the per-attempt reset because it preserves CMake-compatible reload results, but it also notes the reset point is an internal implementation choice rather than a behavior directly mandated by the CMake manual.
  - `design.md` identifies a risk that clearing unresolved load state too aggressively could hide legitimate unresolved diagnostics if a later load attempt fails to set a fresh reason.
  - The current spec and tasks cover stale-state recovery after a successful reload, but they do not require or test that a later failed reload still records the current attempt's unresolved reason after the reset.
  - Remediation: add an error-state scenario and corresponding TDD task proving that a retried file which still fails after reset is left unresolved with a fresh current-attempt reason.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`

### P2(1): Future preset-version maintenance risk is not tracked as a follow-up decision
- Status: Open
- Notes:
  - The best-practice comparison for D1 identifies the remaining maintenance risk that future CMake preset-file versions will require another hand-coded mapping update.
  - `design.md` explicitly rejects a table-driven mapping for this narrow fix, but the artifacts do not capture a follow-up trigger or guardrail for revisiting that decision when CMake adds another preset-file version.
  - Remediation: record a lightweight future-work note or task acceptance criterion that the hand-coded mapping decision should be revisited when a new preset-file version boundary is added.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`
