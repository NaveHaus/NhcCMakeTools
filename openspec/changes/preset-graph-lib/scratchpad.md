## preset-graph-lib Scratchpad

This file tracks the current issue list and working decisions for the `preset-graph-lib` change.
Treat this as the running memory for the refine/clarification process; it is not a spec artifact.

Last updated: 2026-03-26

## Status Legend

- Open: Not yet captured consistently in OpenSpec artifacts.
- Needs refinement: Partially captured; artifacts still need clarifications/consistency work.
- Consistent: Artifacts are aligned with current intended behavior (may still evolve as scope grows).

## Key References

- CMake presets manual: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html

## Current Working Constraints / Decisions

- JSON parsing: standardized on `nlohmann/json` (`nlohmann::json::parse(...)`).
- File IO: `PresetsGraph` gets an injected file loader abstraction (path -> string).
- Include path resolution: relative include paths are resolved relative to the directory of the including presets file.
- Missing include files: non-fatal; keep a File node and mark it Unresolved with reason `FileDoesNotExist`.
- Invalid JSON: non-fatal; keep a File node and mark it Unresolved with reason `InvalidJson`.
- UnresolvedReason: at least `FileDoesNotExist`, `InvalidJson`, `MissingMacro`, `UnsupportedMacro`, `EnvironmentCycle`, `IncludeCycle`, `InheritanceCycle`.
- MacroContext does NOT read the actual process environment.
  - The caller provides both the preset environment map and the parent/process environment map.
- `$env{NAME}` / `$penv{NAME}` behavior:
  - If missing, they remain unresolved (macro text remains), not replaced with an empty string.
  - `$env{}` prefers the preset environment map over the parent environment map.
  - `$penv{}` uses only the parent environment map.
- Builtin/system macros (e.g. `${sourceDir}`, `${hostSystemName}`) do not need to be resolved by this library.
  - If desired, such values must be provided explicitly in MacroContext.

- Preset-associated builtin macros:
  - The graph manager may dynamically populate MacroContext with known preset-associated values during traversal/resolution.
  - Example: `${presetName}` (and other preset-derived values) can be injected from the active preset and its inheritance chain.

- Environment expansion (Option A):
  - The library resolves `environment` map values (which may reference each other) with cycle detection.
  - Missing `$env{}` / `$penv{}` references remain unresolved.

## Issue List

### P0: Manager include-following contract (file loader + JSON parsing)
- Status: Consistent
- Notes:
  - Established file loader abstraction + nlohmann/json parsing contract.
  - Non-fatal missing/invalid files produce Unresolved file nodes.
  - Relative include semantics clarified.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-file-loader/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0: Macro semantics and scope ($env/$penv, preset-specific only)
- Status: Consistent
- Notes:
  - `$env{}` / `$penv{}` are resolved from MacroContext-provided maps.
  - Missing env vars remain unresolved, not empty.
  - Library does not auto-populate system macros; preset-associated values may be injected by the graph manager.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0: Preset model + inheritance resolution (hybrid)
- Status: Needs refinement
- Requirements to capture:
  - Differentiate preset types: ConfigurePreset, BuildPreset, TestPreset, PackagePreset, WorkflowPreset, derived from a base Preset.
  - Each preset stores BOTH:
    - Original/raw JSON fields (as read)
    - Macro-expanded view of macro-expandable string fields
  - Inheritance precedence: earlier entries in `inherits` win for conflicting scalar fields.
  - Environment merge semantics differ (union + null removal + macro rules; cycle constraints).
  - Preset-associated macro injection at least for `${presetName}`, and `${generator}` where applicable.
  - Build/Test/Package `inheritConfigureEnvironment` merge behavior for effective environment.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0: Condition feature set alignment
- Status: Consistent
- Notes:
  - Captures core condition types: const, equals/notEquals, inList/notInList, matches/notMatches, anyOf/allOf/not.

### P0: Cycle handling semantics and error model
- Status: Consistent
- Notes:
  - Include cycles and inheritance cycles are detected and surfaced via UnresolvedReason (`IncludeCycle`, `InheritanceCycle`).

### P1: Target/module naming and packaging
- Status: Consistent
- Notes:
  - Single C++ library `NhcPresetGraph` implements the sub-capabilities as internal modules.

### P2: Spec format consistency
- Status: Open
- Notes:
  - New specs currently use `## ADDED Requirements`. Repo-wide conventions may prefer `## Requirements`.

## Open Questions (Need Clarification)

- Include macro policy in this library:
  - Which system-provided macros (e.g., `${fileDir}`, `${sourceDir}`, `${hostSystemName}`, `${pathListSep}`) will the graph manager populate into MacroContext (if any)?
  - For preset files version 9+, do we treat these macros as supported (when provided) or as `UnsupportedMacro`?
- Preset-associated macro injection:
  - Beyond `${presetName}`, which preset-derived values MUST be injected (e.g., `${generator}` for configure and for build/test via `configurePreset`)?
