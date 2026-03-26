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
- UnresolvedReason: at least `FileDoesNotExist`, `InvalidJson`, `MissingMacro`, `UnsupportedMacro`, `EnvironmentCycle`, `IncludeCycle`, `InheritanceCycle`, `CMakeMinimumRequiredNotMet`, `PresetVersionUnsupported`, `PresetVersionMissing`, `IncludeFieldUnsupportedInPresetVersion`.
- MacroContext does NOT read the actual process environment.
  - The caller provides both the preset environment map and the parent/process environment map.
- `$env{NAME}` / `$penv{NAME}` behavior:
  - If missing, they remain unresolved (macro text remains), not replaced with an empty string.
  - `$env{}` prefers the preset environment map over the parent environment map.
  - `$penv{}` uses only the parent environment map.

- Deviation from CMake:
  - CMake treats missing `$env{NAME}` / `$penv{NAME}` as an empty string; this library keeps them unresolved so the UI can surface them.

- Host/system-provided macros:
  - The library does not query the host system to populate macros like `${hostSystemName}`.
  - If the caller provides `${sourceDir}`, the manager can derive `${sourceParentDir}` and `${sourceDirName}`.

- Preset-associated builtin macros:
  - The graph manager may dynamically populate MacroContext with known preset-associated values during traversal/resolution.
  - Example: `${presetName}` (and other preset-derived values) can be injected from the active preset and its inheritance chain.

- File-associated macros:
  - The graph manager injects file-derived values such as `${fileDir}`.
  - The graph manager injects constant macros such as `${dollar}`.

- Preset availability:
  - Presets track availability as Active/Hidden/Disabled/Unknown.
  - Presets containing `$vendor{...}` are Disabled.

- Root-level/file metadata:
  - Root `version` and `cmakeMinimumRequired` are fields, not macros.
  - Each file node has a preset file `version` field and may have a `cmakeMinimumRequired` constraint.
  - The graph manager is configured with a simulated CMake version.
  - The graph manager computes the maximum supported preset file `version` for the simulated CMake version.
  - The manager validates:
    - missing root `version` => `PresetVersionMissing` (do not process includes/presets)
    - unsupported preset file `version` => `PresetVersionUnsupported`
    - include used with `version` < 4 => `IncludeFieldUnsupportedInPresetVersion`
    - `cmakeMinimumRequired` not satisfied => `CMakeMinimumRequiredNotMet`

  - Observed behavior (local CMake): a top-level `CMakePresets.json` missing root `version` errors even if an included file has a valid `version`.

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
  - Root-level validation matches local CMake behavior:
    - missing root `version` => `PresetVersionMissing` and do not process includes/presets
    - include used with `version` < 4 => `IncludeFieldUnsupportedInPresetVersion`
    - preset file `version` not supported by simulated CMake => `PresetVersionUnsupported`
    - `cmakeMinimumRequired` not satisfied => `CMakeMinimumRequiredNotMet`
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
- Status: Consistent
- Notes:
  - Preset types: Configure/Build/Test/Package/Workflow.
  - Raw JSON retained plus expanded view for macro-expandable fields used by the library.
  - Inheritance precedence: earlier `inherits` entries win scalar conflicts.
  - Environment merge semantics: union + `null` removal, with `inheritConfigureEnvironment` ordering.
  - Preset-specific macros supported: `${presetName}`, `${generator}`.
  - Environment values expand with cycle detection (`EnvironmentCycle`).
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
- Status: Consistent
- Notes:
  - OpenSpec change specs use delta headers (e.g., `## ADDED Requirements`) by design.

## Open Questions (Need Clarification)

(None currently)
