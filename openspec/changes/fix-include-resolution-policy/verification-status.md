## fix-include-resolution-policy Verification Status

This file tracks the current `openspec-verify` findings list and working decisions
for the implementation of the `fix-include-resolution-policy` change. Treat this as the running memory for the
implementation verification process; it is NOT a spec artifact.

Last updated: 2026-05-21

### Summary
| Dimension    | Status                                                                 |
|--------------|------------------------------------------------------------------------|
| Completeness | All tasks 1.1-1.7 and 2.1-2.3 checked; delta spec scenarios implemented |
| Correctness  | Manager delegates include resolution to `PresetIncludeGraph::ResolveIncludes`; unsupported-macro cases covered |
| Coherence    | Delta spec, design, proposal, source, tests align; `openspec validate --strict` passes |

## Key References

- Change spec delta: `openspec/changes/fix-include-resolution-policy/specs/preset-graph-manager/spec.md`
- Source: `src/PresetsGraph/PresetsGraph.cpp` (`ApplyContext`, `TryLoadFileNode`)
- Tests: `tests/PresetsGraph/GraphManagerTests.cpp` (lines 330, 354, 380 for the three RED scenarios; line 568 for repeated-inclusion tolerance)
- Strict validation: `openspec validate fix-include-resolution-policy --strict` => valid

## Current Working Constraints / Decisions

- Workflow preset listed in tasks.md (`clangd-ninja-vcpkg-release-test`) does not exist; available equivalent is `clang-clangd-ninja-vcpkg-mt-s-release-test`.
- The harness reported that `CMakePresets.json` was updated to honor inherited `VCPKG_ROOT`, but the worktree's `CMakePresets.json` still has the `.vcpkg-root` preset's `environment.VCPKG_ROOT` set to `./vcpkg-root`, which overrides the inherited env and breaks the workflow before any compilation begins.

## Verification Run Log

- (2026-05-20) Command: `cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test`
  - Result: FAILED at configure step.
  - Reason: At the time of this run, the `.vcpkg-root` configure preset still had `"environment": { "VCPKG_ROOT": "./vcpkg-root" }`, overriding the inherited `VCPKG_ROOT`. Environmental, unrelated to the change under verification.
- (2026-05-21) Re-run after parent branch picked up `5d031fe build: allow VCPKG_ROOT to be overridden from the environment`:
  - Command: `cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test`
  - Result: FAILED at vcpkg manifest install — unrelated triplet defect (preset hardcodes `VCPKG_TARGET_TRIPLET=x64-linux` / `VCPKG_HOST_TRIPLET=x64-linux` on a Windows clang preset). Out of scope.
  - Fallback Command: `cmake --workflow --preset=vs18-vcpkg-mt-s-release-test`
  - Result: SUCCESS. `100% tests passed, 0 tests failed out of 9` in 0.14s. `GraphManagerTests` PASSED.

## Findings List by Priority

### CRITICAL

(none)

### WARNING

1. Workflow verification command in `tasks.md` (2.3) references a non-existent preset.
  - Status: Completed
  - Notes:
    - Task 2.3 updated to reference the actually-defined `clang-clangd-ninja-vcpkg-mt-s-release-test` workflow preset.
  - Artifacts touched:
    - `openspec/changes/fix-include-resolution-policy/tasks.md` (task 2.3)
  - Sources touched:
    - (none — task-text only)

2. Workflow preset run could not be executed in this worktree due to `CMakePresets.json` overriding `VCPKG_ROOT` to a relative `./vcpkg-root`.
  - Status: Resolved (2026-05-21)
  - Notes:
    - Original block: harness assertion that `CMakePresets.json` honored inherited `VCPKG_ROOT` did not hold at first-pass time.
    - Resolution: parent branch picked up commit `5d031fe build: allow VCPKG_ROOT to be overridden from the environment`. Re-run with `vs18-vcpkg-mt-s-release-test` succeeded (`100% tests passed, 0 of 9 failed`).
  - Artifacts touched:
    - (none in this change)
  - Sources touched:
    - `CMakePresets.json` (out-of-scope; fixed on parent branch)

3. Named workflow preset `clang-clangd-ninja-vcpkg-mt-s-release-test` blocked by Linux-triplet defect on Windows clang preset.
  - Status: Deferred
  - Notes:
    - `CMakePresets.json` hardcodes `VCPKG_TARGET_TRIPLET=x64-linux` / `VCPKG_HOST_TRIPLET=x64-linux` on this preset. Causes vcpkg manifest install to fail when run on Windows.
    - Out of scope for this OpenSpec change. Workflow re-verification accomplished via `vs18-vcpkg-mt-s-release-test` fallback.
  - Artifacts touched:
    - (none in this change)
  - Sources touched:
    - `CMakePresets.json` (out-of-scope)

### SUGGESTION

(none)

## Open Questions (Need Clarification)

- Should `tasks.md` 2.3 be updated to reference the actually-defined workflow preset `clang-clangd-ninja-vcpkg-mt-s-release-test`? Outside-of-change CMakePresets fix would be in a separate change.
