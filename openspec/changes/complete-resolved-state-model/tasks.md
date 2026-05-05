## 1. Resolved-state test coverage

- [ ] 1.1 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for preset-owned expanded environment entries with per-entry resolution status.
- [ ] 1.2 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for unresolved environment references being preserved in resolved environment entries.
- [ ] 1.3 Add or update `tests/PresetsGraph/PresetModelTests.cpp` scenarios that fail for additional scalar resolved-state coverage such as `cmakeExecutable`.
- [ ] 1.4 Register any new or renamed preset-model tests in `tests/PresetsGraph/CMakeLists.txt` if needed and run the focused preset-model test target in the RED state.

## 2. Preset-owned resolved-state implementation

- [ ] 2.1 Update `PresetModel` resolved-state refresh logic to store expanded effective environment entries as preset-owned resolved fields with stable per-key field naming.
- [ ] 2.2 Update `PresetModel` resolved-state refresh logic to preserve `FullyResolved`, `PartiallyExpanded`, and unresolved status per environment entry.
- [ ] 2.3 Extend the scalar field allowlist used for resolved-state expansion to cover the next set of library-relevant string fields required by the new tests.
- [ ] 2.4 Keep one shared internal merge and expansion pipeline so preset-owned resolved fields and compatibility paths do not diverge.
- [ ] 2.5 Build and run the focused preset-model tests until the GREEN state is reached.

## 3. Compatibility and verification

- [ ] 3.1 Migrate current internal callers that still depend on the public `ResolvedPreset` path to use preset-owned resolved fields where the replacement path is complete.
- [ ] 3.2 Mark the public `ResolvedPreset` path as compatibility-only or deprecated in the implementation comments and API surface without removing it yet.
- [ ] 3.3 Refactor any duplicated resolved-state logic revealed by the migration while keeping behavior unchanged.
- [ ] 3.4 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full build and test suite after the TDD cycle completes.
