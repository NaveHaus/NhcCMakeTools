## complete-resolved-state-model Scratchpad

Tracks openspec-refine issues and working decisions for `complete-resolved-state-model` artifacts.
This is a working document, not a spec artifact.

Last updated: 2026-05-05

## Status Legend
- **Open**: Not yet captured consistently in OpenSpec artifacts.
- **Needs refinement**: Partially captured; artifacts still need clarifications/consistency work.
- **Consistent**: Artifacts are aligned with current intended behavior (may still evolve as scope grows).
- **Accepted**: User accepted the affected artifact(s) as-is.

## Key References
- CMake 4.3.2 `cmake-presets(7)` manual: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
  - User-directed best-practice focus for this refinement round.
  - Exa search/fetch tools were required by the user but returned "not available or not permitted" in this session, so manual content could not be retrieved through Exa.
- `openspec/specs/preset-model/spec.md`
- `openspec/specs/preset-macro-context/spec.md`
- `openspec/changes/archive/2026-04-29-preset-graph-lib/design.md`

## Current Working Constraints / Decisions
- Write scope is limited to `openspec/changes/complete-resolved-state-model/`.
- No commit will be created in this refinement round.
- Best-practice search was narrowed by the user to CMake preset behavior in the CMake 4.3.2 `cmake-presets(7)` manual.
- Because Exa retrieval was unavailable, no research-driven design change was applied beyond the safe CMake preset field-name correction below.

## Best-Practice Comparison
| Decision | vs. Industry Standard | Alternatives | Gaps |
|----------|----------------------|--------------|------|
| D1: Preset-owned resolved state remains the single authoritative model | Aligned. The CMake manual defines resolved behavior in terms of applying `inherits`, include reachability rules, condition enablement, and macro expansion within the preset being used, not as a separate public resolved-document type. Keeping resolved state attached to the owning preset matches that execution model and avoids inventing a second top-level contract that CMake itself does not expose. | Considered: keep `ResolvedPreset` as a peer public contract. | The comparison supports the direction, but the artifacts should keep the boundary explicit: authoritative resolved data is derived from preset semantics, not from raw JSON snapshots. |
| D2: Expanded environment entries are stored as per-key resolved fields | Mostly aligned. In CMake, `environment` is a map keyed by variable name, inherits by union across parents, permits `null` to suppress inherited values, and allows entry-to-entry macro references with cycle rejection. A per-key resolved model matches that behavior better than a single opaque resolved blob because the merge and suppression semantics are defined at environment-variable granularity. | Considered: one resolved JSON object for the full environment map. Additional user-choice alternatives: key entries as `environment.<name>`, key entries as `environment[name]`, or store a nested resolved `environment` object with per-entry metadata. | The artifacts still need one stable naming convention for resolved environment entries so that per-key inheritance, `null` suppression, and expansion status can be represented without ambiguity. |
| D3: Scalar field coverage expands by a maintained allowlist | Aligned with CMake's schema-driven field set. The manual enumerates preset fields explicitly, and some string fields have distinct semantics such as path resolution (`binaryDir`, `toolchainFile`, `installDir`, `graphviz`), reserved IDE-only usage (`cmakeExecutable`), or structured string maps (`cacheVariables`, `environment`). A maintained allowlist is safer than auto-expanding every string-valued raw JSON field because it respects those field-specific semantics and future schema growth. | Considered: expand every string-valued raw JSON field automatically. | The artifacts should keep the allowlist criteria explicit so newly added CMake fields are reviewed against manual semantics before entering the resolved scalar model. |
| D4: Public `ResolvedPreset` migration is staged, not immediate | Aligned with compatibility-preserving API evolution. The manual describes additive preset-format growth across versions and field-specific behavior changes, which argues for a staged migration while downstream callers move from the legacy public type to preset-owned resolved state. | Considered: remove `ResolvedPreset` immediately. | No material gap in the comparison itself; the remaining work is migration planning and call-site cleanup rather than a design mismatch with CMake preset behavior. |

## Issue List

### P0(1): Environment resolved-entry key convention remains unspecified
- Status: Consistent
- Notes:
  - The spec requires resolved-state keys to use CMake preset field names and also requires one tracked resolved entry per environment key.
  - `tasks.md` required stable per-key field naming, but no artifact had chosen the convention.
  - Resolved by selecting the nested `environment` object convention: the resolved-state object stores a single `environment` key whose value is a nested object keyed by environment variable name, each entry carrying its expanded value and resolution status.
  - Flattened keys (`environment.<name>`) and bracketed keys (`environment[<name>]`) were rejected in `design.md` with rationale.
- Artifacts touched:
  - `openspec/changes/complete-resolved-state-model/design.md` — Decision 2 updated with convention, rationale, and rejected alternatives
  - `openspec/changes/complete-resolved-state-model/specs/preset-model/spec.md` — resolved-state requirement updated to specify nested `environment` key convention
  - `openspec/changes/complete-resolved-state-model/tasks.md` — Task 2.1 updated to reference nested `environment` key convention

### P1(1): Additional scalar example used a non-preset field name
- Status: Consistent
- Notes:
  - The artifacts used `cmakeGeneratorPlatform` as the example of additional scalar resolved-state coverage.
  - This does not match the CMake preset field names used by the local preset-model artifacts.
  - The example was replaced with `cmakeExecutable`, a CMake preset field name that fits the maintained allowlist direction.
- Artifacts touched:
  - `openspec/changes/complete-resolved-state-model/design.md`
  - `openspec/changes/complete-resolved-state-model/tasks.md`
  - `openspec/changes/complete-resolved-state-model/specs/preset-model/spec.md`

### P1(2): Scalar allowlist inclusion criteria remain implicit
- Status: Consistent
- Notes:
  - The best-practice comparison for D3 says newly added CMake fields should be reviewed against manual semantics before entering the resolved scalar model.
  - No artifact had defined the criteria used to decide whether a field belongs in the maintained scalar allowlist.
  - Resolved by adding five explicit inclusion criteria to `design.md` Decision 3: (1) defined as a string field by the CMake manual, (2) accepts macro expansion, (3) not a structured map or array, (4) not reserved exclusively for IDE/tooling consumers, (5) the library already reads the field or it is required by a new test scenario. Fields failing any criterion require a design note to be added.
  - `tasks.md` Task 2.3 updated to reference the criteria explicitly so future implementors apply the gate during allowlist extension.
- Artifacts touched:
  - `openspec/changes/complete-resolved-state-model/design.md` — Decision 3 updated with five explicit allowlist inclusion criteria and a constraint on exceptions
  - `openspec/changes/complete-resolved-state-model/tasks.md` — Task 2.3 updated to reference the five criteria
