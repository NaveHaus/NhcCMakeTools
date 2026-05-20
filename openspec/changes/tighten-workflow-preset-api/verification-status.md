## tighten-workflow-preset-api Verification Status

This file tracks the current `openspec-verify` findings list and working decisions
for the implementation of the `tighten-workflow-preset-api` change. Treat this as the running memory for the
implementation verification process; it is NOT a spec artifact.

Last updated: 2026-05-20

### Summary
| Dimension    | Status                                  |
|--------------|-----------------------------------------|
| Completeness | 7/7 tasks, 1/1 modified req implemented |
| Correctness  | 1/1 req covered, all scenarios covered  |
| Coherence    | Followed (1 doc-only issue)             |

## Key References

- `src/PresetsGraph/PresetModel.h` lines 165-187 — `WorkflowPreset` with private `using` declarations hiding `ClearCondition`, `GetCondition`, `GetConditionState`, `GetEnvironment`, `GetHidden`, `GetInherits`, `SetCondition`, `SetConditionExplicitNull`, `SetEnvironment`, `SetHidden`, `SetInherits`.
- `tests/PresetsGraph/PresetModelTests.cpp` lines 53-132 — `HasGetConditionState`, `HasSetConditionExplicitNull`, `HasClearCondition` concept-based static_asserts on `WorkflowPreset` confirm accessors are not callable.
- `CMakePresets.json` workflowPresets: `vs18-vcpkg-mt-s-release-test`, `clang-clangd-ninja-vcpkg-mt-s-release-test`. No preset named `clangd-ninja-vcpkg-release-test` exists.

## Current Working Constraints / Decisions

- Doc-only fix: tasks.md task 3.2 references a workflow preset that does not exist in `CMakePresets.json`. Rename to the matching actual preset `clang-clangd-ninja-vcpkg-mt-s-release-test`.

## Findings List by Priority

### CRITICAL

None.

### WARNING

1. `tasks.md` task 3.2 references workflow preset `clangd-ninja-vcpkg-release-test`, which does not exist in `CMakePresets.json`. Actual workflow presets are `vs18-vcpkg-mt-s-release-test` and `clang-clangd-ninja-vcpkg-mt-s-release-test`. The clang-clangd workflow is the intended verification target.
   - Status: Completed
   - Notes:
     - Doc-only correction; implementation and spec are unaffected.
     - Renamed to `clang-clangd-ninja-vcpkg-mt-s-release-test`.
   - Artifacts touched:
     - `openspec/changes/tighten-workflow-preset-api/tasks.md`
   - Sources touched:
     - (none)

### SUGGESTION

None.

## Open Questions (Need Clarification)

- None.
