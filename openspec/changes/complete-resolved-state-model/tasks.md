## 1. Resolved-state test coverage

- [x] 1.1 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for preset-owned expanded environment entries with per-entry resolution status.
- [x] 1.2 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for unresolved environment references being preserved in resolved environment entries.
- [x] 1.3 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for additional scalar resolved-state coverage such as `cmakeExecutable`.
- [x] 1.4 Register any new or renamed preset-model tests in `tests/PresetsGraph/CMakeLists.txt` if needed and run the focused preset-model test target in the RED state.

## 2. Preset-owned resolved-state implementation

- [x] 2.1 Update `PresetModel` resolved-state refresh logic to store expanded effective environment entries as preset-owned resolved fields under a nested `environment` key, with each environment variable name as a sub-key carrying its expanded value and resolution status.
- [x] 2.2 Update `PresetModel` resolved-state refresh logic to preserve `FullyResolved`, `PartiallyExpanded`, and unresolved status per environment entry.
- [x] 2.3 Extend the scalar field allowlist used for resolved-state expansion to cover the next set of library-relevant string fields required by the new tests, verifying each candidate against the five allowlist inclusion criteria in the design before adding it.
- [x] 2.4 Keep one shared internal merge and expansion pipeline so preset-owned resolved fields and compatibility paths do not diverge.
- [x] 2.5 Build and run the focused preset-model tests until the GREEN state is reached.

## 3. Compatibility and verification

- [x] 3.1 Migrate current internal callers that still depend on the public `ResolvedPreset` path to use preset-owned resolved fields where the replacement path is complete.
- [x] 3.2 Mark the public `ResolvedPreset` path as compatibility-only or deprecated in the implementation comments and API surface without removing it yet.
- [x] 3.3 Refactor any duplicated resolved-state logic revealed by the migration while keeping behavior unchanged.
- [x] 3.4 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full build and test suite after the TDD cycle completes.
