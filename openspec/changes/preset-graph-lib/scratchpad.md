## preset-graph-lib Scratchpad

This file tracks the current issue list and working decisions for the `preset-graph-lib` change.
Treat this as the running memory for the refine/clarification process; it is not a spec artifact.

Last updated: 2026-03-26

## Key References

- CMake presets manual: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html

## Current Working Constraints / Decisions

- JSON parsing: standardized on `nlohmann/json` (`nlohmann::json::parse(...)`).
- File IO: `PresetsGraph` gets an injected file loader abstraction (path -> string).
- Include path resolution: relative include paths are resolved relative to the directory of the including presets file.
- Missing include files: non-fatal; keep a File node and mark it Unresolved with reason `FileDoesNotExist`.
- Invalid JSON: non-fatal; keep a File node and mark it Unresolved with reason `InvalidJson`.
- UnresolvedReason: at least `FileDoesNotExist`, `InvalidJson`, `MissingMacro`, `UnsupportedMacro`.
- MacroContext does NOT read the actual process environment.
  - The caller provides both the preset environment map and the parent/process environment map.
- `$env{NAME}` / `$penv{NAME}` behavior:
  - If missing, they remain unresolved (macro text remains), not replaced with an empty string.
  - `$env{}` prefers the preset environment map over the parent environment map.
  - `$penv{}` uses only the parent environment map.
- Builtin/system macros (e.g. `${sourceDir}`, `${hostSystemName}`) do not need to be resolved by this library.
  - If desired, such values must be provided explicitly in MacroContext.

## Issue List

### P0: Manager include-following contract (file loader + JSON parsing)
- Status: In-progress (captured in artifacts; implementation not started)
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
- Status: In-progress (captured in artifacts; implementation not started)
- Notes:
  - `$env{}` / `$penv{}` are resolved from MacroContext-provided maps.
  - Missing env vars remain unresolved, not empty.
  - Library does not auto-populate builtin/system macros.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0: Preset model + inheritance resolution (hybrid)
- Status: Open (next to specify)
- Requirements to capture:
  - Differentiate preset types: ConfigurePreset, BuildPreset, TestPreset, PackagePreset, WorkflowPreset, derived from a base Preset.
  - Each preset stores BOTH:
    - Original/raw JSON fields (as read)
    - Macro-expanded view of macro-expandable string fields
  - Inheritance precedence: earlier entries in `inherits` win for conflicting scalar fields.
  - Environment merge semantics differ (union + null removal + macro rules; cycle constraints).

### P0: Condition feature set alignment
- Status: Open
- Notes:
  - Current condition AST spec is a minimal subset; confirm intended operators and parsing scope.

### P0: Cycle handling semantics and error model
- Status: Open
- Notes:
  - Need a consistent, queryable diagnostic model for include cycles and inheritance cycles.

### P1: Target/module naming and packaging
- Status: Open
- Notes:
  - Proposal lists sub-capabilities; tasks mention a single `NhcPresetGraph` library. Decide final packaging/naming.

### P2: Spec format consistency
- Status: Open
- Notes:
  - New specs currently use `## ADDED Requirements`. Repo-wide conventions may prefer `## Requirements`.

## Open Questions (Need Clarification)

- Environment self-references: should env map values be expanded with references to other env vars (and with cycle detection), or should they be retained for the UI to show without attempting to fully resolve?
- Include macro policy in this library: given builtin/system macros are out of scope, which macros do we explicitly support in include strings beyond `$penv{}`?
