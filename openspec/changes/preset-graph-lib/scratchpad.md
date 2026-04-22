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
- End-to-end ingestion SHALL inspect `configurePresets`, `buildPresets`, `testPresets`, `packagePresets`, and `workflowPresets`, replace per-file preset contributions on reload, and populate the inheritance graph only from preset types that support inheritance.
- Condition handling SHALL distinguish field absence, explicit `condition: null`, and parsed condition ASTs; explicit `null` clears inherited conditions for the current preset and is not inherited by descendants.

## Best-Practice Comparison
| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|-------------|------|
| D1: Dual DAGs | Aligned | Considered | The artifacts now capture `CMakeUserPresets.json` implicit inclusion as a directional user-to-project include relationship for v1. |
| D2: Topology vs Payload | Aligned | Considered | Stable file identity via normalized absolute paths is only partially propagated from design into normative specs/tasks. |
| D3: Structural vs Cosmetic State | Aligned | Considered | The artifacts now state the v1 compatibility boundary: structural rules follow CMake, while macro expansion remains diagnostic-friendly where documented. |
| D4: Condition AST | Aligned | Considered | The artifacts now cover the CMake condition wire format, including boolean/null/object parsing, explicit-null inheritance semantics, and parse-failure reporting. |
| D5: Retaining Disabled Nodes | Aligned | Considered | The CMake manual says presets containing `$vendor{}` are ignored; the UI-facing “Disabled” representation is reasonable but should be framed more explicitly as a library-level interpretation. |
| D6: Graceful Partial Macro Expansion | Aligned | Considered | The proposal, design, and specs now bound this behavior explicitly as a diagnostic-friendly library contract rather than strict CMake emulation. |
| D7: File Loader + `nlohmann/json` | Aligned | Considered | File loading, version enforcement, and JSON-to-model ingestion are now captured; remaining work is implementation-oriented. |
| D8: Non-Fatal Resolution Diagnostics | Aligned | Considered | The non-fatal diagnostic model fits interactive tooling well, but the artifacts do not yet distinguish project-provided includes from user-local includes as the CMake docs do. |
| D9: Preset Model Layer | Aligned | Considered | Workflow step compatibility and non-fatal validation diagnostics are now captured in design/spec/tasks for v1. |

## Issue List

### P0(1): End-to-end preset ingestion is under-specified
- Status: Consistent
- Notes:
  - `design.md` now defines a JSON-to-model ingestion policy covering all supported root preset arrays, per-file replacement semantics, and the boundary between model storage and inheritance-graph participation.
  - `specs/preset-graph-manager/spec.md` now normatively requires ingestion of `configurePresets`, `buildPresets`, `testPresets`, `packagePresets`, and `workflowPresets` into `PresetModel`.
  - `specs/preset-graph-manager/spec.md` also now requires re-ingesting a file to refresh that file's preset contribution instead of appending duplicates, and it defines that Workflow presets remain model-only for inheritance purposes.
  - `tasks.md` now includes explicit RED/GREEN coverage for root-array ingestion, per-file refresh, and inheritance-graph population from typed presets.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P0(2): CMake condition parsing semantics are incomplete
- Status: Consistent
- Notes:
  - `design.md` now defines condition parsing in terms of the CMake wire format and explicitly distinguishes absent condition fields, explicit `null`, and parsed AST values.
  - `specs/preset-condition-ast/spec.md` now normatively covers boolean/null/object parsing, supported object member names, and parse-failure reporting.
  - `specs/preset-model/spec.md` and `specs/preset-inheritance-graph/spec.md` now capture the inheritance semantics of explicit `condition: null`, including that it clears the current preset's inherited condition and is not inherited by descendants.
  - `specs/preset-graph-manager/spec.md` now requires parsing condition fields during preset ingestion and reporting malformed conditions as `InvalidCondition`.
  - `tasks.md` now includes explicit RED/GREEN coverage for JSON condition parsing, explicit-null inheritance behavior, and manager-level invalid-condition diagnostics.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-condition-ast/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-inheritance-graph/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
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
- Status: Consistent
- Notes:
  - The CMake manual treats `CMakePresets.json` and `CMakeUserPresets.json` as first-class roots with important behavioral differences.
  - `CMakeUserPresets.json` implicitly includes `CMakePresets.json`, and inheritance from `CMakePresets.json` into `CMakeUserPresets.json` is directional.
  - User clarification: v1 SHALL support `CMakeUserPresets.json` by auto-including `CMakePresets.json` only when that file exists at the same relative path.
  - `proposal.md` and `design.md` now describe the v1 user-root behavior explicitly.
  - `specs/preset-graph-manager/spec.md` and `specs/preset-include-graph/spec.md` now normatively define the sibling auto-include rule and its one-way direction.
  - `tasks.md` now includes explicit RED/GREEN coverage for implicit user-root inclusion and the no-sibling case.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-include-graph/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1(2): Workflow preset interaction rules are under-specified
- Status: Consistent
- Notes:
  - `specs/preset-model/spec.md` captures workflow step shape (`type` + `name`) but not CMake’s workflow constraints.
  - The CMake manual requires the first workflow step to be `configure`, and all subsequent steps to be non-configure presets whose `configurePreset` matches the initial configure preset.
  - `design.md` now defines workflow-step compatibility as a v1 policy and frames violations as non-fatal diagnostics.
  - `specs/preset-model/spec.md` and `specs/preset-graph-manager/spec.md` now normatively define workflow validation behavior and diagnostics.
  - `tasks.md` now includes explicit RED/GREEN coverage for first-step validation, configure-preset matching, and workflow diagnostics.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-graph-manager/spec.md`
  - `openspec/changes/preset-graph-lib/tasks.md`

### P1(3): The strict-CMake vs diagnostic-library boundary is unclear
- Status: Consistent
- Notes:
  - `design.md` and the macro/preset specs intentionally keep missing `$env{}` / `$penv{}` references unresolved so the UI can surface missing inputs.
  - The proposal previously overstated the contract as strict CMake accuracy without clarifying the interactive diagnostic boundary.
  - User clarification: the library SHALL expose diagnostic-friendly macro semantics only; strict CMake emulation is not the contract for v1.
  - `proposal.md` and `design.md` now state that v1 follows CMake structural rules while preserving diagnostic-friendly macro behavior where documented.
  - `specs/preset-macro-context/spec.md` and `specs/preset-model/spec.md` now normatively bind unresolved macro handling as library-defined behavior rather than strict CMake emulation.
- Artifacts touched:
  - `openspec/changes/preset-graph-lib/proposal.md`
  - `openspec/changes/preset-graph-lib/design.md`
  - `openspec/changes/preset-graph-lib/specs/preset-macro-context/spec.md`
  - `openspec/changes/preset-graph-lib/specs/preset-model/spec.md`

## Open Questions (Need Clarification)
- None currently.
