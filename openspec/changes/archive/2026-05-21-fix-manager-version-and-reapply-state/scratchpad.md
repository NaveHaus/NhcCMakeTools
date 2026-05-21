## fix-manager-version-and-reapply-state Scratchpad

Tracks openspec-refine issues and working decisions for `fix-manager-version-and-reapply-state` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-05

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
- Exa web search/fetch tools were unavailable in the prior session; confirmed available and used in the 2026-05-05 session.
- Best-practice comparison for D1 and D2 verified against the live CMake 4.3.2 `cmake-presets(7)` manual (fetched 2026-05-05): version 10 is listed as "Added in version 3.31" and version 11 as "Added in version 4.3".

## Best-Practice Comparison

| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|--------------|------|
| D1: Treat CMake 4.0 through 4.2 as version-10 managers | **Verified against live CMake 4.3.2 docs (2026-05-05).** The manual explicitly lists version `10` as "Added in version 3.31" and version `11` as "Added in version 4.3". The `design.md` boundary is correct: simulated CMake `4.0`–`4.2` report version `10`, and `4.3` is the first release that reports `11`. | Considered in `design.md`: keep `major >= 4`; table-driven mapping. The `major >= 4` alternative conflicts with the manual. A table-driven mapping would also align, but is broader than this change requires. | Maintenance risk: future preset-file versions will require another hand-coded update. Addressed in `design.md` risk entry with a table-driven refactor trigger for version 12+. |
| D2: Clear unresolved load state at the start of each reload attempt | **Verified against live CMake 4.3.2 docs (2026-05-05).** Consistent with the observable behavior implied by the manual: preset loading is based on the current file set and current reload attempt's inputs, not stale diagnostics from an earlier failed attempt. | Considered in `design.md`: rebuild include graph on every apply; add bulk graph reset API. Both alternatives eliminate stale state but go beyond what the change requires. | The manual specifies observable preset semantics, not internal node-lifecycle rules. The artifact rationale is inferential. Failed-reload behavior after reset was not originally specified; addressed in spec.md with a new normative clause and scenario (2026-05-05). |

## Issue List

### P1(1): CMake preset best-practice comparison blocked by unavailable Exa tools
- Status: Consistent
- Notes:
  - Prior session: Exa tools were unavailable; comparison was performed using model training knowledge.
  - 2026-05-05: Exa tools confirmed available. Live CMake 4.3.2 `cmake-presets(7)` manual fetched and verified.
  - Manual confirms: version 10 → "Added in version 3.31"; version 11 → "Added in version 4.3".
  - Both D1 and D2 are consistent with the authoritative CMake documentation. Best-Practice Comparison table updated to reflect live-doc verification.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`

### P1(2): Retry failure diagnostics after unresolved-state reset are not specified
- Status: Consistent
- Notes:
  - `design.md` identified a risk that clearing unresolved load state too aggressively could hide legitimate unresolved diagnostics if a later load attempt fails to set a fresh reason.
  - The prior spec covered stale-state recovery only for the success case.
  - 2026-05-05: Added normative clause to the `Reload Attempts Recompute File Unresolved State` requirement in `spec.md`: failed reloads MUST leave the node Unresolved with the current-attempt reason, not any prior reason.
  - Added scenario `Failed reload after reset records current-attempt reason` to `spec.md`.
  - Added tasks 2.4 RED and 2.5 GREEN to `tasks.md` to drive TDD coverage of this scenario.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/specs/preset-graph-manager/spec.md`
  - `openspec/changes/fix-manager-version-and-reapply-state/tasks.md`
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`

### P2(1): Future preset-version maintenance risk is not tracked as a follow-up decision
- Status: Consistent
- Notes:
  - `design.md` rejected a table-driven mapping for this narrow fix but did not capture a trigger for revisiting that decision.
  - 2026-05-05: Updated the maintenance risk entry in `design.md` to state that when CMake introduces a preset-file version beyond 11, the hand-coded boundary logic SHOULD be reconsidered in favor of the table-driven mapping approach.
- Artifacts touched:
  - `openspec/changes/fix-manager-version-and-reapply-state/design.md`
  - `openspec/changes/fix-manager-version-and-reapply-state/scratchpad.md`

## Open Questions (Need Clarification)
- (None)
