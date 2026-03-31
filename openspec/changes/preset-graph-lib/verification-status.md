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

**Affected Files**:
- `src/PresetsGraph/PresetModel.h:42`
- `src/PresetsGraph/PresetModel.cpp:82`

**Suggestion**:
Add raw/original preset storage plus raw-and-expanded field accessors so the model retains the original JSON and preserves original strings alongside computed expansions.

### Minimal typed preset view omits required fields

**Severity**: CRITICAL

**Affected Files**:
- `src/PresetsGraph/PresetModel.h:42`

**Suggestion**:
Extend the typed preset model so callers can query `inherits`, `condition`, `environment`, `configurePreset`, and `inheritConfigureEnvironment` in addition to the currently exposed fields.

### Missing notEquals condition support

**Severity**: CRITICAL

**Affected Files**:
- `src/PresetsGraph/Condition.h:45`
- `src/PresetsGraph/Condition.cpp:80`
- `tests/PresetsGraph/ConditionTests.cpp`

**Suggestion**:
Implement `NotEqualsCondition` and add BDD test coverage for the corresponding requirement scenarios.

### Missing anyOf and not condition support

**Severity**: CRITICAL

**Affected Files**:
- `src/PresetsGraph/Condition.h:109`
- `src/PresetsGraph/Condition.cpp:145`
- `tests/PresetsGraph/ConditionTests.cpp`

**Suggestion**:
Implement `AnyOfCondition` and `NotCondition`, then add tests that cover the required boolean logic scenarios.

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
