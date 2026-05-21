## complete-resolved-state-model Verification Status

This file tracks the current `openspec-verify` findings list and working decisions
for the implementation of the `complete-resolved-state-model` change. Treat this as the running memory for the
implementation verification process; it is NOT a spec artifact.

Last updated: 2026-05-21

### Summary
| Dimension    | Status                                  |
|--------------|-----------------------------------------|
| Completeness | 13/13 tasks, 2 reqs covered             |
| Correctness  | 2/2 reqs implemented; workflow blocked  |
| Coherence    | Design followed; one stale preset name  |

## Key References

- `openspec/changes/complete-resolved-state-model/specs/preset-model/spec.md` - delta requirements
- `openspec/changes/complete-resolved-state-model/tasks.md` - task list
- `src/PresetsGraph/PresetModel.h` - `ResolvedPreset` marked compatibility-only (lines 189-204)
- `src/PresetsGraph/PresetModel.cpp` - resolved-state refresh with environment nesting and per-entry status (lines 535-575, 77-86, 103-114)
- `tests/PresetsGraph/PresetModelTests.cpp` - test coverage for environment/cmakeExecutable scenarios
- `CMakePresets.json` - workflow presets `vs18-vcpkg-mt-s-release-test`, `clang-clangd-ninja-vcpkg-mt-s-release-test`

## Current Working Constraints / Decisions

- Worktree base commit: `0602a2d` (parent branch tip `claude/cmake-presets-graph-design-6Eo8t`).
- Upstream commit `5d031fe` (VCPKG_ROOT env override) is NOT in the parent branch; `CMakePresets.json` still hardcodes `VCPKG_ROOT=./vcpkg-root`.
- File-edit permissions in this worktree are scoped to `openspec/changes/**` and `.claude/worktrees/**`. CMakePresets.json at worktree root cannot be modified; `git cherry-pick` is not in the allowlist either.
- Per task instructions, NO `vcpkg-root` junction may be created.
- Workflow re-run was attempted with the correct existing preset `clang-clangd-ninja-vcpkg-mt-s-release-test`. It failed at the configure step because the hardcoded `./vcpkg-root` toolchain path does not resolve (env override is ignored without `5d031fe`).

## Findings List by Priority

### CRITICAL

(none)

### WARNING

1. Stale workflow preset name in tasks.md task 3.4
  - Status: Completed
  - Notes:
    - tasks.md line 21 referenced `cmake --workflow --preset=clangd-ninja-vcpkg-release-test`.
    - `cmake --list-presets=workflow` reports only `vs18-vcpkg-mt-s-release-test` and `clang-clangd-ninja-vcpkg-mt-s-release-test`.
    - Renamed in tasks.md to `clang-clangd-ninja-vcpkg-mt-s-release-test`.
  - Artifacts touched:
    - `openspec/changes/complete-resolved-state-model/tasks.md`
  - Sources touched, using relative paths:
    - (none)

2. Workflow re-run could not be executed in this verification round
  - Status: Deferred
  - Notes:
    - Configure failed with `Could not find toolchain file: "./vcpkg-root/scripts/buildsystems/vcpkg.cmake"`.
    - Root cause is environmental: parent branch is missing commit `5d031fe` (VCPKG_ROOT env override). Worktree permissions disallow editing `CMakePresets.json` and cherry-picking; instructions forbid creating the `vcpkg-root` junction.
    - This is not a defect in the `complete-resolved-state-model` change itself. The change's own implementation (preset-owned resolved state, environment nesting, allowlist, compatibility marker) is in place per source inspection.
    - Recommend re-running `cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test` once `5d031fe` lands on the parent branch.
  - Artifacts touched:
    - (none)
  - Sources touched, using relative paths:
    - `CMakePresets.json` (cannot edit in this worktree)

### SUGGESTION

(none)

## Open Questions (Need Clarification)

- None.
