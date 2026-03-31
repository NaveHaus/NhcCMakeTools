## Verification Status: `preset-graph-lib`

### Summary

| Dimension | Status |
|---|---|
| Completeness | 123/123 tasks complete; 48 requirements reviewed |
| Correctness | ~44/48 requirements clearly evidenced; full build/test workflow passed |
| Coherence | Core design mostly followed; a few implementation/design mismatches found |

- Verification run: `cmake --workflow --preset=clangd-ninja-vcpkg-mt-s-release-test`
- Result: 8/8 tests passed
- Final assessment: 4 critical issues found; fix before archiving

## Issues

### Raw and expanded preset views are incomplete

**Severity**: CRITICAL

**Status**: Fixed

**Affected Files**:
- `src/PresetsGraph/PresetModel.h:42`
- `src/PresetsGraph/PresetModel.cpp:82`

**Suggestion**:
Add raw/original preset storage plus raw-and-expanded field accessors so the model retains the original JSON and preserves original strings alongside computed expansions.

**Resolution**: Added `GetPreset(name)` accessor to expose raw `Preset` values. The dual-view design (raw via `GetPreset()`, resolved via `ResolvePreset()`) is now documented in design.md Decision #9. Tasks 6a.18-6a.20 added and completed with TDD.

### Minimal typed preset view omits required fields

**Severity**: CRITICAL

**Status**: Artifacts Updated (Implementation Pending)

**Affected Files**:
- `src/PresetsGraph/PresetModel.h:42`

**Suggestion**:
Extend the typed preset model so callers can query `inherits`, `condition`, `environment`, `configurePreset`, and `inheritConfigureEnvironment` in addition to the currently exposed fields.

**Resolution**: Updated design.md Decision #9 with "Preset Type Hierarchy" section specifying a class hierarchy (base `Preset` with derived `ConfigurePreset`, `BuildPreset`, `TestPreset`, `PackagePreset`, `WorkflowPreset`). Revised the workflow-preset artifact language so `WorkflowPreset` MAY derive from `Preset` for implementation convenience, but its typed API exposes only `name` and `steps` and omits unsupported accessors. Updated specs/preset-model/spec.md and tasks 6b.11-6b.12 to match. Implementation pending.

### Missing notEquals condition support

**Severity**: CRITICAL

**Status**: Fixed

**Affected Files**:
- `src/PresetsGraph/Condition.h:45`
- `src/PresetsGraph/Condition.cpp:80`
- `tests/PresetsGraph/ConditionTests.cpp`

**Suggestion**:
Implement `NotEqualsCondition` and add BDD test coverage for the corresponding requirement scenarios.

**Resolution**: Implemented `NotEqualsCondition` class following the `EqualsCondition` pattern. Added scenarios to specs/preset-condition-ast/spec.md. Added tasks 4.18-4.21 and completed with TDD. Two BDD tests added for true/false cases.

### Missing anyOf and not condition support

**Severity**: CRITICAL

**Status**: Fixed

**Affected Files**:
- `src/PresetsGraph/Condition.h:109`
- `src/PresetsGraph/Condition.cpp:145`
- `tests/PresetsGraph/ConditionTests.cpp`

**Suggestion**:
Implement `AnyOfCondition` and `NotCondition`, then add tests that cover the required boolean logic scenarios.

**Resolution**: Implemented `AnyOfCondition` (short-circuit true, all-false, and unknown handling) and `NotCondition` (inversion with unknown preservation). Added scenarios to specs/preset-condition-ast/spec.md. Added tasks 4.22-4.34 and completed with TDD. Seven BDD tests added covering all logic cases.

### Condition parsing uses a placeholder AST

**Severity**: WARNING

**Affected Files**:
- `src/PresetsGraph/PresetsGraph.cpp:318`

**Suggestion**:
Replace the placeholder `EqualsCondition("${missingMacro}", "x")` with real parsing of the preset `condition` JSON into the appropriate AST nodes.

### Manager only loads configure presets

**Severity**: WARNING

**Affected Files**:
- `src/PresetsGraph/PresetsGraph.cpp:296`

**Suggestion**:
Load `buildPresets`, `testPresets`, `packagePresets`, and `workflowPresets` as well, or revise the change artifacts to explicitly narrow the supported scope.

### File loader scenarios lack dedicated verification coverage

**Severity**: WARNING

**Affected Files**:
- `src/PresetsGraph/FileLoader.h:18`
- `tests/PresetsGraph/GraphManagerTests.cpp:20`

**Suggestion**:
Add focused BDD tests that verify successful absolute-path loading and missing-file failure reporting for the file loader abstraction.

### File node identity is not normalized to absolute paths

**Severity**: SUGGESTION

**Affected Files**:
- `src/PresetsGraph/IncludeGraph.cpp:231`
- `src/PresetsGraph/PresetsGraph.cpp:109`

**Suggestion**:
Normalize file node paths to absolute paths when nodes are created if stable identity across reloads is required by the intended UI behavior.
