## Why

Unsupported include macros are currently diagnosed incorrectly when include resolution runs through the graph manager. The include graph already defines the policy CMake requires, but the manager bypasses that path, so callers receive `MissingMacro` or silently expanded results where `UnsupportedMacro` is required.

## What Changes

- Clarify the manager's include-resolution contract so `ApplyContext()` delegates include macro-policy enforcement to the include graph instead of re-implementing expansion rules.
- Preserve the current layering: include macro validation stays in the include graph, while the manager remains responsible for iterative file discovery and loading.
- Add implementation tasks and test coverage for unsupported include macros observed through the manager integration path.

## Capabilities

### New Capabilities

(None)

### Modified Capabilities

- `preset-graph-manager`: `ApplyContext()` must use include-graph resolution results so unsupported include macros are surfaced with the include graph's `UnsupportedMacro` diagnostics during manager orchestration.

## Impact

- **Code**: `src/PresetsGraph/PresetsGraph.cpp` integration logic and adjacent include-resolution plumbing.
- **Tests**: Manager-level include resolution coverage in `tests/PresetsGraph`.
- **Behavior**: Unsupported include macros are reported consistently with the include graph spec during manager-driven resolution.
