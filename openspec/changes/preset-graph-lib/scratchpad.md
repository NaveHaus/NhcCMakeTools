## preset-graph-lib Scratchpad

Tracks openspec-refine issues and working decisions for `preset-graph-lib` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-04-21

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake Presets manual, latest: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- CMake Presets manual, CMake 3.31: https://cmake.org/cmake/help/v3.31/manual/cmake-presets.7.html
- CMake CLI workflow mode, latest: https://cmake.org/cmake/help/latest/manual/cmake.1.html

## Current Working Constraints / Decisions
- Review scope narrowed to CMake-specific preset semantics, especially include rules, condition forms, preset interaction, and inheritance precedence.
- The repository baseline is CMake 3.31+, but the artifacts intentionally model preset-version support through CMake 4.3 / preset schema version 11.
- The current design intentionally favors UI-visible diagnostics over strict CMake emulation for missing `$env{}` / `$penv{}` values.
- v1 SHALL handle `CMakeUserPresets.json` by automatically including `CMakePresets.json`, but only when `CMakePresets.json` can be found at the same relative path.
- The library contract SHALL remain diagnostic-friendly rather than strict-CMake for macro expansion semantics because the primary consumer is a dynamic UI.

## Best-Practice Comparison
| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Dual DAGs | Aligned | Considered | Missing explicit handling for `CMakeUserPresets.json` implicit inclusion and user-vs-project include provenance from the CMake spec. |
| D2: Topology vs Payload | Aligned | Considered | Stable file identity via normalized absolute paths is only partially propagated from design into normative specs/tasks. |
| D3: Structural vs Cosmetic State | Room for improvement | Considered | The UI-oriented state model is coherent, but the artifacts do not clearly define the compatibility boundary with strict CMake evaluation semantics. |
| D4: Condition AST | Room for improvement | Considered | The artifacts cover AST evaluation but not the full CMake condition wire format, especially boolean/null forms and parser responsibilities. |
| D5: Retaining Disabled Nodes | Aligned | Considered | The CMake manual says presets containing `$vendor{}` are ignored; the UI-facing “Disabled” representation is reasonable but should be framed more explicitly as a library-level interpretation. |
| D6: Graceful Partial Macro Expansion | Room for improvement | Considered | The diagnostic-first deviation from CMake empty-string behavior is documented in design/spec, but not clearly bounded in proposal scope or compatibility language. |
| D7: File Loader + `nlohmann/json` | Room for improvement | Considered | File loading/version enforcement is specified, but end-to-end parsing of preset arrays into the typed model/graphs is not. |
| D8: Non-Fatal Resolution Diagnostics | Aligned | Considered | The non-fatal diagnostic model fits interactive tooling well, but the artifacts do not yet distinguish project-provided includes from user-local includes as the CMake docs do. |
| D9: Preset Model Layer | Room for improvement | Considered | The type hierarchy aligns with CMake preset categories, but workflow step semantics and full configure/build/test/package interactions remain under-specified. |

## Issue List

### P0(1): End-to-end preset ingestion is under-specified
- Status: Open
- Notes:
  - `proposal.md` and `design.md` describe parsing raw preset files into a typed model that drives graph resolution.
  - `specs/preset-graph-manager/spec.md` currently stops at file loading, version checks, include handling, and composite state.
  - `tasks.md` does not include explicit RED/GREEN coverage for parsing `configurePresets`, `buildPresets`, `testPresets`, `packagePresets`, and `workflowPresets` from loaded JSON into `PresetModel` and the graph structures.
  - This leaves a major implementation-readiness gap between “load JSON” and “evaluate actual CMake preset collections”.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0(2): CMake condition parsing semantics are incomplete
- Status: Open
- Notes:
  - `design.md` says preset `condition` values will be parsed into an AST.
  - `specs/preset-condition-ast/spec.md` and `tasks.md` only cover evaluating hand-constructed AST nodes.
  - The CMake preset manual allows `condition` to be a boolean, `null`, or an object, and `null` has specific inheritance semantics.
  - The artifacts do not define how JSON condition values are parsed, how `null` is represented, or how those semantics propagate through inheritance.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-inheritance-graph/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0(3): PresetModel resolved-state architecture overfits a synthetic resolved object
- Status: Consistent
- Notes:
  - User review identified that a separate `ResolvedPreset` / `RawResolvedPreset` design does not match the intended preset-model architecture.
  - The artifacts now require each preset to own both its raw JSON and its current resolved field state.
  - The resolved state is now described as a field-oriented mapping keyed by CMake preset field names, with per-field resolution status and JSON support for structured fields such as `cacheVariables`.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1(1): `CMakeUserPresets.json` behavior is not modeled or explicitly deferred
- Status: Open
- Notes:
  - The CMake manual treats `CMakePresets.json` and `CMakeUserPresets.json` as first-class roots with important behavioral differences.
  - `CMakeUserPresets.json` implicitly includes `CMakePresets.json`, and inheritance from `CMakePresets.json` into `CMakeUserPresets.json` is directional.
  - User clarification: v1 SHALL support `CMakeUserPresets.json` by auto-including `CMakePresets.json` only when that file exists at the same relative path.
  - The current artifacts still do not capture that rule normatively or in tasks.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1(2): Workflow preset interaction rules are under-specified
- Status: Open
- Notes:
  - `specs/preset-model/spec.md` captures workflow step shape (`type` + `name`) but not CMake’s workflow constraints.
  - The CMake manual requires the first workflow step to be `configure`, and all subsequent steps to be non-configure presets whose `configurePreset` matches the initial configure preset.
  - No artifact currently defines validation behavior, diagnostics, or task coverage for these constraints.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1(3): The strict-CMake vs diagnostic-library boundary is unclear
- Status: Open
- Notes:
  - `design.md` and the macro/preset specs intentionally keep missing `$env{}` / `$penv{}` references unresolved so the UI can surface missing inputs.
  - The proposal simultaneously claims the library will “accurately represent the constraints of the CMake specification”.
  - User clarification: the library SHALL expose diagnostic-friendly macro semantics only; strict CMake emulation is not the contract for v1.
  - The tradeoff is defensible, but the artifacts still do not state that boundary clearly enough.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`

## Open Questions (Need Clarification)
- None currently.
