## preset-graph-lib Scratchpad

This file tracks the current openspec-refine issue list and working decisions
for the `preset-graph-lib` change. Treat this as the running memory for the
refine/clarification process; it is NOT a spec artifact.

Last updated: 2026-03-26

## Status Legend
- Open: Not yet captured consistently in OpenSpec artifacts.
- Needs refinement: Partially captured; artifacts still need clarifications/consistency work.
- Consistent: Artifacts are aligned with current intended behavior (may still evolve as scope grows).

## Key References

- `openspec/changes/preset-graph-lib/proposal.md`
- `openspec/changes/preset-graph-lib/design.md`
- `openspec/changes/preset-graph-lib/tasks.md`
- `openspec/changes/preset-graph-lib/specs/`
- `AGENTS.md`

## Current Working Constraints / Decisions

- OpenSpec artifacts must remain internally consistent across proposal/design/specs/tasks.
- Tests must be registered via `nhc_add_test_executable(... USES NhcTestLib)` in `tests/PresetsGraph/CMakeLists.txt` per `AGENTS.md`.
- Macro expansion must not read host system environment by default.
- Best-practices review is deferred for this refinement step.

## Issue List

### P0: preset-model capability missing from scope artifacts
- Status: Consistent
- Notes:
  - Resolved: `proposal.md` lists `preset-model` and `design.md` includes a dedicated Preset Model decision.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`

### P0: test registration tasks missing required CMake steps
- Status: Consistent
- Notes:
  - Resolved: `tasks.md` includes explicit tasks for `tests/PresetsGraph/CMakeLists.txt` and wiring it into the parent tests CMake configuration.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1: include-macro allowance is underspecified
- Status: Consistent
- Notes:
  - Resolved: `specs/preset-include-graph/spec.md` defines v7/v8 vs v9+ allow/reject rules, plus MissingMacro vs UnsupportedMacro behavior.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`

### P1: design decisions missing alternatives
- Status: Consistent
- Notes:
  - Resolved: Decisions 2, 3, 5, 6, 8 now include explicit Alternatives Considered.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`

### P2: partial expansion reporting is non-testable
- Status: Consistent
- Notes:
  - Resolved: `specs/preset-macro-context/spec.md` defines ExpansionResult with Status and UnresolvedTokens.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`

## Open Questions (Need Clarification)
- (None)
