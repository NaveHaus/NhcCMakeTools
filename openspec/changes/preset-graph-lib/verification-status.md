## preset-graph-lib Verification Status

This file tracks the current openspec-verify findings list and working decisions
for the implementation of the `preset-graph-lib` change. Treat this as the
running memory for the implementation verification process; it is NOT a spec
artifact.

Last updated: 2026-04-29

### Summary

| Dimension    | Status                  |
|--------------|-------------------------|
| Completeness | 166/207 tasks, 64 reqs  |
| Correctness  | 57/64 reqs covered      |
| Coherence    | Followed with issues    |

## Status Legend

- Unresolved: The implementation does not match the change artifacts.
- In progress: The implementation is being updated to remediate the finding.
- Deferred: The finding will be fixed in a subsequent change.
- Completed: The implementation has been made **Complete**, **Correct**, and **Coherent**

## Verification Snapshot

- OpenSpec status: `spec-driven`; artifacts report `done`.
- OpenSpec task progress: 166/207 tasks complete; 41 tasks remaining.
- Requirements reviewed: 64 requirements across 8 delta spec files.
- Last verification commands:
  - `openspec status --change "preset-graph-lib" --json`
  - `openspec instructions apply --change "preset-graph-lib" --json`
  - `rg -c "^- \[x\]" openspec/changes/preset-graph-lib/tasks.md`
  - `rg -c "^- \[ \]" openspec/changes/preset-graph-lib/tasks.md`
- CMake workflow verification was not rerun during this sync.

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
- `src/PresetsGraph/PresetModel.h`
- `src/PresetsGraph/PresetModel.cpp`
- `src/PresetsGraph/PresetsGraph.cpp`
- `tests/PresetsGraph/ConditionTests.cpp`
- `tests/PresetsGraph/GraphManagerTests.cpp`
- `tests/PresetsGraph/PresetModelTests.cpp`

## Current Working Constraints / Decisions

- The change is not ready to archive while `tasks.md` contains unchecked tasks.
- The current design requires parsed condition wire forms, explicit `condition: null`
  semantics, typed ingestion for all supported preset arrays, per-file preset refresh,
  workflow validation, and raw JSON plus resolved state on each preset instance.
- Current source still uses a placeholder condition AST during manager ingestion.
- Current source still ingests only `configurePresets` in `PresetsGraph::TryLoadFileNode`.
- Current source still exposes `ResolvedPreset` and `RawResolvedPreset` as separate
  model-managed resolved snapshots.

## Findings List

### CRITICAL

1. Condition JSON wire-format parsing is not implemented.
  - Status: Unresolved
  - Notes:
    - Tasks 4.35-4.43 remain unchecked.
    - The condition AST supports several evaluation node types, but the JSON parser
      entry point and object dispatch required by the spec are still missing.
    - Manager ingestion still substitutes a synthetic placeholder condition instead
      of parsing boolean, object, null, or invalid wire forms.
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
  - Status: Unresolved
  - Notes:
    - Tasks 6a.6c-6a.6f remain unchecked.
    - The design requires explicit null to clear inherited conditions for the current
      preset without becoming an inheritable condition for descendants.
    - This depends on preserving the distinction between absent condition, explicit
      null, and parsed condition AST.
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
  - Status: Unresolved
  - Notes:
    - Tasks 6c.1-6c.8 remain unchecked.
    - `PresetModel` still exposes a separate `ResolvedPreset` return object and an
      internal `RawResolvedPreset` helper in `src/PresetsGraph/PresetModel.h`.
    - The design requires raw JSON and per-field resolved state to live on each
      preset instance using CMake field names and expansion status.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetModel.h`
    - `src/PresetsGraph/PresetModel.cpp`
    - `tests/PresetsGraph/PresetModelTests.cpp`

4. Root `CMakeUserPresets.json` implicit sibling include handling is missing.
  - Status: Unresolved
  - Notes:
    - Tasks 7.2ta-7.2tc remain unchecked.
    - The include graph and manager specs require a root `CMakeUserPresets.json` to
      auto-include a sibling `CMakePresets.json` when present, without synthesizing
      the include when the sibling file is absent.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
    - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/PresetsGraph.cpp`
    - `tests/PresetsGraph/GraphManagerTests.cpp`

5. Preset file ingestion only handles `configurePresets`.
  - Status: Unresolved
  - Notes:
    - Tasks 7.2u-7.2aa remain unchecked.
    - `PresetsGraph::TryLoadFileNode` currently inspects only `configurePresets`.
    - The design requires ingestion of `configurePresets`, `buildPresets`,
      `testPresets`, `packagePresets`, and `workflowPresets`, with per-file refresh
      semantics and correct inheritance-graph projection.
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
  - Status: Unresolved
  - Notes:
    - Tasks 7.2ab-7.2ah remain unchecked.
    - `PresetsGraph::TryLoadFileNode` still assigns `EqualsCondition("${missingMacro}", "x")`
      whenever a preset contains a `condition` field.
    - Invalid condition syntax is not surfaced as `InvalidCondition`, so manager
      state cannot accurately represent condition parse failures.
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
  - Status: Unresolved
  - Notes:
    - Tasks 7.2ai-7.2ak remain unchecked.
    - The model defines `WorkflowPreset` and `WorkflowStep`, but manager validation
      for first-step configure requirements and configure-preset compatibility is
      not implemented.
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
    - The current remaining issue is not absence of the type hierarchy; it is that
      resolved state still lives outside the preset instance.
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
    - This completion does not include JSON wire-format parsing for those condition
      nodes.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/tasks.md`
    - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/Condition.h`
    - `src/PresetsGraph/Condition.cpp`
    - `tests/PresetsGraph/ConditionTests.cpp`

### WARNING

1. File loader behavior lacks focused test coverage separate from graph manager tests.
  - Status: Unresolved
  - Notes:
    - Existing manager tests exercise file loading through `PresetsGraph`, but the
      file loader abstraction still lacks dedicated BDD coverage for absolute-path
      success and missing-file failure reporting.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/specs/preset-file-loader/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/FileLoader.h`
    - `tests/PresetsGraph/GraphManagerTests.cpp`

### SUGGESTION

1. File node identity is not normalized to absolute paths.
  - Status: Unresolved
  - Notes:
    - The design says include graph file nodes should store normalized absolute
      paths for stable identity across reloads.
    - Current include resolution uses lexical normalization, but root and created
      node paths are not consistently forced to absolute paths.
  - Artifacts touched:
    - `openspec/changes/preset-graph-lib/design.md`
    - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - Sources touched, using relative paths:
    - `src/PresetsGraph/IncludeGraph.cpp`
    - `src/PresetsGraph/PresetsGraph.cpp`

## Open Questions (Need Clarification)

- None for the current verification-status sync.
