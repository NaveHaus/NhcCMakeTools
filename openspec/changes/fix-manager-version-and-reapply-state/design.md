## Context

The current architecture review identified two manager-level defects in `PresetsGraph`. First, the supported preset file version mapping treats every simulated CMake 4.x release as if it supports preset file version 11, even though the CMake 4.3.2 documentation establishes that version 11 begins at CMake 4.3. Second, repeated `ApplyContext()` calls can leave a file node marked unresolved after a successful reload because the prior unresolved load reason is never cleared before the next load attempt.

This change is intentionally narrow. It is limited to manager orchestration behavior and does not redefine include-graph macro policy, inheritance behavior, or the typed preset model.

## Goals / Non-Goals

**Goals:**
- Align the simulated CMake-to-preset-file-version mapping with the documented CMake 4.3 boundary.
- Ensure repeated `ApplyContext()` calls recompute file load state from the current reload attempt rather than preserving stale unresolved diagnostics.
- Keep the change localized to manager behavior and document it with testable OpenSpec scenarios.

**Non-Goals:**
- Refactor include resolution ownership between `PresetsGraph` and `PresetIncludeGraph`.
- Change preset-model APIs or inheritance resolution behavior.
- Expand this fix into the adjacent architecture-review issues that are not part of the requested change.

## Decisions

### 1. Treat CMake 4.0 through 4.2 as version-10 managers

The manager will map preset file version 11 only when the simulated version is greater than 4.2, while CMake 4.0 through 4.2 continue to report version 10 support.

This matches the architecture review and CMake 4.3 documentation without changing any earlier 3.x mappings.

Alternatives considered:
- Continue using `major >= 4` for version 11 support. Rejected because it is factually incorrect for 4.0, 4.1, and 4.2.
- Generalize the mapping into a table-driven refactor now. Rejected because the requested change is small and does not require broader rework.

### 2. Clear unresolved load state at the start of each reload attempt

The reload path should clear `IsUnresolved` and `Reason` on the target file node before trying to load or parse the file again. This keeps the node's diagnostic state tied to the current attempt instead of the previous one.

The reset belongs at the start of the manager's file-load path rather than in global graph teardown. The manager already owns retry timing, and a per-attempt reset avoids mutating unrelated nodes.

Alternatives considered:
- Rebuild the include graph from scratch on every `ApplyContext()` call. Rejected because it is broader than necessary and would change graph identity semantics.
- Add a bulk graph reset API and call it before every apply. Rejected because the bug is specific to file-load retry state, not all include-graph payload state.

## Risks / Trade-offs

- Boundary logic can regress again if future CMake releases add a new preset version and the mapping remains hand-coded. Mitigation: add explicit boundary scenarios for both 4.2 and 4.3 in the spec and tests.
- Clearing unresolved load state too aggressively could hide legitimate unresolved diagnostics if the current load attempt fails to set a new reason. Mitigation: keep the reset scoped to the file currently being retried and preserve the existing failure paths that assign fresh reasons.
