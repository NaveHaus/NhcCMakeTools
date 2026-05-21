## Why

The preset graph manager currently overstates preset file version support for simulated CMake 4.0 through 4.2 and can retain stale unresolved include-file state across repeated `ApplyContext()` calls. Both behaviors produce incorrect diagnostics and prevent the manager from matching the CMake 4.3 version boundary and its own reload expectations.

## What Changes

- Correct the simulated CMake-to-preset-version mapping so preset file version 11 is only supported for CMake 4.3 and newer.
- Define manager behavior for repeated `ApplyContext()` calls so a file node's unresolved load state is cleared before the manager retries loading that file.
- Add specification scenarios that cover the CMake 4.2 versus 4.3 boundary and stale unresolved-state recovery after a successful reload.

## Capabilities

### New Capabilities

(None.)

### Modified Capabilities

- `preset-graph-manager`: Tighten simulated CMake version support rules and require repeated context application to clear stale file-node unresolved state before a reload attempt.

## Impact

- **Code**: `src/PresetsGraph/PresetsGraph.cpp` load/reload orchestration and supported-version logic.
- **Tests**: `tests/PresetsGraph/GraphManagerTests.cpp` coverage for CMake 4.x version mapping and repeated `ApplyContext()` recovery.
- **Behavior**: Manager diagnostics and resolved state become consistent with the CMake 4.3 preset-version boundary and reload outcomes.
