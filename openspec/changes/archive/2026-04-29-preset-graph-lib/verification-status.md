## preset-graph-lib Verification Status

This file tracks the current openspec-verify findings list and working decisions
for the implementation of the `preset-graph-lib` change. Treat this as the
running memory for the implementation verification process; it is NOT a spec
artifact.

Last updated: 2026-04-29

### Summary

| Dimension    | Status                  |
|--------------|-------------------------|
| Completeness | 207/207 tasks, 64 reqs |
| Correctness  | 64/64 reqs covered     |
| Coherence    | Followed               |

## Status Legend

- Unresolved: The implementation does not match the change artifacts.
- In progress: The implementation is being updated to remediate the finding.
- Deferred: The finding will be fixed in a subsequent change.
- Completed: The implementation has been made **Complete**, **Correct**, and **Coherent**

## Verification Snapshot

- OpenSpec status: `spec-driven`; artifacts report `done`.
- OpenSpec task progress: 207/207 tasks complete; 0 tasks remaining.
- Requirements reviewed: 64 requirements across 8 delta spec files.
- Last verification commands:
  - `openspec status --change "preset-graph-lib" --json`
  - `openspec instructions apply --change "preset-graph-lib" --json`
  - `rg -c "^- \[x\]" openspec/changes/preset-graph-lib/tasks.md`
  - `rg -c "^- \[ \]" openspec/changes/preset-graph-lib/tasks.md`
  - `cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test`
- CMake workflow verification passed with 9/9 tests passing.
- 2026-04-29 update-mode verification found no new implementation/artifact
  mismatches.

## Key References

- `openspec/changes/preset-graph-lib/design.md`
- `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
- `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
- `openspec/changes/preset-graph-lib/specs/preset-inheritance-graph/spec.md`
- `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
- `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
- `openspec/changes/preset-graph-lib/tasks.md`
- `src/PresetsGraph/Condition.h`
- `src/PresetsGraph/Condition.cpp`
- `src/PresetsGraph/FileLoader.h`
- `src/PresetsGraph/FileLoader.cpp`
- `src/PresetsGraph/IncludeGraph.cpp`
- `src/PresetsGraph/PresetModel.h`
- `src/PresetsGraph/PresetModel.cpp`
- `src/PresetsGraph/PresetsGraph.h`
- `src/PresetsGraph/PresetsGraph.cpp`
- `tests/PresetsGraph/ConditionTests.cpp`
- `tests/PresetsGraph/FileLoaderTests.cpp`
- `tests/PresetsGraph/GraphManagerTests.cpp`
- `tests/PresetsGraph/IncludeGraphTests.cpp`
- `tests/PresetsGraph/PresetModelTests.cpp`

## Current Working Constraints / Decisions

- `tasks.md` has no unchecked tasks remaining.
- `ResolvePreset()` remains as a compatibility snapshot API, but raw JSON and
  current resolved field state now live on each preset instance.
- The manager now parses condition wire forms during ingestion, ingests all
  supported preset arrays, refreshes per application pass, validates workflows,
  and records invalid condition diagnostics.
- Include graph file identity is normalized to absolute paths.

## Findings List by Priority

### CRITICAL

1. Condition JSON wire-format parsing is not implemented.
  - Status: Completed
  - Notes:
    - Tasks 4.35-4.43 are checked.
    - `ParseConditionJson()` parses boolean, null, and typed object wire forms.
    - Manager ingestion now uses parsed conditions instead of a placeholder AST.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/Condition.h`
    - `src/PresetsGraph/Condition.cpp`
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `tests/PresetsGraph/ConditionTests.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`

2. Explicit `condition: null` inheritance semantics are not implemented in preset resolution.
  - Status: Completed
  - Notes:
    - Tasks 6a.6c-6a.6f are checked.
    - `PresetConditionState` preserves absent, explicit null, and expression states.
    - `PresetModel::ResolveCondition()` clears inherited conditions for explicit
      null and does not propagate the null marker to descendants.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-inheritance-graph/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `src/PresetsGraph/InheritanceGraph.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`
    - `tests/PresetsGraph/InheritanceGraphTests.cpp`

3. Presets do not yet own raw JSON plus current resolved field state.
  - Status: Completed
  - Notes:
    - Tasks 6c.1-6c.8 are checked.
    - Each preset now stores raw JSON and a CMake-field-name keyed resolved field
      map with per-field expansion status.
    - `RawResolvedPreset` was replaced; `ResolvePreset()` remains only as a
      compatibility snapshot API.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`

4. Root `CMakeUserPresets.json` implicit sibling include handling is missing.
  - Status: Completed
  - Notes:
    - Tasks 7.2ta-7.2tc are checked.
    - `PresetsGraph` adds a directional implicit include edge to a readable sibling
      `CMakePresets.json` and does not synthesize an edge when the sibling is absent.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`

5. Preset file ingestion only handles `configurePresets`.
  - Status: Completed
  - Notes:
    - Tasks 7.2u-7.2aa are checked.
    - `PresetsGraph::TryLoadFileNode` ingests `configurePresets`, `buildPresets`,
      `testPresets`, `packagePresets`, and `workflowPresets`.
    - Workflow presets remain model-only; configure/build/test/package presets
      populate the inheritance graph.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`

6. Manager-level condition ingestion and diagnostics are missing.
  - Status: Completed
  - Notes:
    - Tasks 7.2ab-7.2ah are checked.
    - Manager ingestion stores parsed boolean/object/null conditions.
    - Invalid condition syntax marks the preset unresolved with `InvalidCondition`
      and makes the manager state unresolved.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/Condition.h`
    - `src/PresetsGraph/Condition.cpp`
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `tests/PresetsGraph/ConditionTests.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`

7. Workflow preset validation is missing.
  - Status: Completed
  - Notes:
    - Tasks 7.2ai-7.2ak are checked.
    - The manager validates first-step configure requirements and subsequent
      build/test/package configure-preset compatibility.
    - Workflow diagnostics are non-fatal and the workflow preset remains queryable.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`

8. Raw preset access and the minimal typed preset hierarchy are implemented.
  - Status: Completed
  - Notes:
    - Previous findings for missing raw access and missing common typed fields have
      been superseded by completed tasks 6a.18-6a.20 and 6b.1-6b.23.
    - Resolved state now lives on the preset instance as part of completed tasks
      6c.1-6c.8.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`

9. `notEquals`, `anyOf`, and `not` condition evaluation nodes are implemented.
  - Status: Completed
  - Notes:
    - Previous findings for missing condition node support have been resolved by
      completed tasks 4.18-4.34.
    - JSON wire-format parsing for these nodes is now included in completed tasks
      4.35-4.43.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/Condition.h`
    - `src/PresetsGraph/Condition.cpp`
    - `tests/PresetsGraph/ConditionTests.cpp`

### WARNING

1. File loader behavior lacks focused test coverage separate from graph manager tests.
  - Status: Completed
  - Notes:
    - Dedicated `FileLoaderTests` cover absolute-path success and missing-file
      failure reporting.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/specs/preset-file-loader/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/FileLoader.h`
    - `src/PresetsGraph/FileLoader.cpp`
    - `tests/PresetsGraph/FileLoaderTests.cpp`

### SUGGESTION

1. File node identity is not normalized to absolute paths.
  - Status: Completed
  - Notes:
    - Include graph node identity now uses normalized absolute paths for roots and
      created include nodes.
    - Include and manager tests were updated to assert absolute-path identity.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/IncludeGraph.cpp`
    - `src/PresetsGraph/PresetsGraph.cpp`

## Open Questions (Need Clarification)

- None for the current verification-status sync.
