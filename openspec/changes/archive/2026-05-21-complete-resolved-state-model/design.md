## Context

`preset-model` already specifies that resolved state lives on each preset and preserves per-field resolution status. The current implementation still exposes a separate public `ResolvedPreset` path, expands only a small subset of scalar fields into preset-owned resolved state, and records merged `environment` data without expanding each value or preserving per-entry status. The architecture review identifies these as the remaining resolved-state gaps for `preset-model`.

This change is intentionally limited to OpenSpec artifacts. It defines the implementation direction without modifying callers, source, or tests in this turn.

## Goals / Non-Goals

**Goals:**

- Make preset-owned resolved fields the authoritative resolved-state interface for `preset-model`.
- Require environment values to be expanded into preset-owned resolved state with one tracked resolved entry per environment key.
- Preserve partial and unresolved expansion results exactly, including unresolved environment references.
- Extend resolved scalar coverage incrementally to additional library-relevant CMake preset string fields instead of keeping the current four-field special case.
- Define a migration path away from the public `ResolvedPreset` API without forcing a flag day.

**Non-Goals:**

- Removing the public `ResolvedPreset` API in this change.
- Defining full resolved-state coverage for every possible raw JSON field in one step.
- Changing include-graph, inheritance-graph, or workflow behavior outside the resolved-state surface.
- Performing implementation, test, or caller migration work in this artifact-only turn.

## Decisions

### 1. Preset-owned resolved state remains the single authoritative model

The spec delta will reinforce that callers are expected to use preset-owned resolved fields rather than a separate model-managed resolved object. This matches the existing spec direction and keeps per-field status attached to the preset that owns the data.

Alternative considered:
- Keep `ResolvedPreset` as a peer public contract. Rejected because it duplicates merge logic, omits per-field status, and drifts from the spec.

### 2. Expanded environment entries are stored as per-key resolved fields under a nested `environment` resolved field

Resolved state will track each effective environment entry independently after inheritance merge and environment expansion. Resolved environment entries are stored under a single `environment` key in the resolved-state object, using a nested object keyed by environment variable name. Each nested entry carries the expanded value and its resolution status. This mirrors the raw JSON `environment` field shape, avoids encoding concerns with dots or brackets in variable names, and keeps the resolved-state structure consistent with other structured fields such as `cacheVariables`.

This allows one key to be `FullyResolved` while another remains `PartiallyExpanded`, which is not possible if the whole environment block is treated as one opaque fully resolved object.

Alternatives considered:
- Store one resolved JSON object for the full environment map. Rejected because it collapses status to the map level and loses the per-entry diagnostic behavior required by the spec.
- Flattened keys such as `environment.<name>`. Rejected because dots are valid in some environment variable names on certain platforms, introducing ambiguity at the key level.
- Bracketed keys such as `environment[<name>]`. Rejected because bracket syntax has no established precedent in the codebase and requires escaping for variable names that contain brackets.

### 3. Scalar field coverage expands by a maintained allowlist with explicit inclusion criteria

The implementation direction should grow resolved scalar coverage through an explicit list of library-relevant CMake preset field names rather than trying to infer expandability dynamically from arbitrary JSON. This stays consistent with existing code structure while allowing incremental additions such as `cmakeExecutable`.

A CMake preset string field is eligible for the scalar allowlist when it meets ALL of the following criteria:
1. It is defined as a string-typed field by the CMake `cmake-presets(7)` manual for at least one preset type.
2. It accepts macro expansion in CMake (i.e., the manual does not describe it as a verbatim or passthrough value).
3. It is not a structured map or array — fields such as `cacheVariables` and `environment` are handled by their own resolved-state mechanisms.
4. It is not reserved exclusively for IDE or tooling consumers (e.g., `cmakeExecutable` is in scope; hypothetical IDE-only vendor extension fields are not).
5. The library already reads or evaluates the field, or the field is required by a new test scenario in this change.

Fields that do not meet all five criteria MUST NOT be added to the scalar allowlist without an explicit design note justifying the exception.

Alternative considered:
- Expand every string-valued field from raw JSON automatically. Rejected because it would blur the boundary between typed fields and opaque passthrough JSON, and it risks changing behavior for fields the library does not yet evaluate.

### 4. Public `ResolvedPreset` migration is staged, not immediate

The design assumes a staged migration:
1. Make preset-owned resolved state complete enough for current callers.
2. Move internal and external callers to `RefreshResolvedState()` plus preset-owned resolved-field access.
3. Deprecate the public `ResolvedPreset` path once callers no longer need it.

Alternative considered:
- Remove `ResolvedPreset` immediately. Rejected because it would expand scope into a breaking API cleanup before the replacement path is complete.

## Risks / Trade-offs

- Expanded scalar coverage may still be incomplete after the first implementation pass -> Mitigation: explicitly scope the initial allowlist in tasks and extend it only for fields already supported by the library.
- Environment entry keying could become inconsistent if the implementation does not define a stable naming convention for resolved fields -> Mitigation: require a documented convention in the implementation and test it directly.
- Staged migration leaves duplicate resolved-state paths temporarily -> Mitigation: keep one shared internal merge/expansion pipeline and treat `ResolvedPreset` as compatibility-only.
- Per-entry environment tracking increases test surface -> Mitigation: use focused TDD scenarios for fully resolved, partially resolved, inherited, and cycle-detected environment cases.
