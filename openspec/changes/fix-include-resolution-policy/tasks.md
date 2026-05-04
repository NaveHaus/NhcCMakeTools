## 1. Manager include-policy delegation

- [ ] 1.1 **RED**: Add a failing manager-level test in `tests/PresetsGraph/GraphManagerTests.cpp` verifying that an include using `$env{HOME}` is reported as `UnsupportedMacro` when resolved through `ApplyContext()`.
- [ ] 1.2 **RED**: Add a failing manager-level test verifying that preset-specific include macros such as `${presetName}` remain `UnsupportedMacro` through the manager path.
- [ ] 1.3 **RED**: Add a failing manager-level test verifying that a version 7 or 8 file using `${fileDir}` in `include` is reported as `UnsupportedMacro` through the manager path.
- [ ] 1.4 **GREEN**: Refactor `PresetsGraph::ApplyContext()` so per-file include resolution delegates macro-policy enforcement to `PresetIncludeGraph` while preserving file-derived macro injection and iterative discovery.
- [ ] 1.5 **GREEN**: Update manager/include-graph integration plumbing so newly resolved file paths continue to load and refresh correctly after delegation.
- [ ] 1.6 **REFACTOR**: Review the manager/include-graph boundary for duplicated include-policy logic and remove any remaining redundant checks.

## 2. Verification

- [ ] 2.1 Build the affected manager test target after the RED/GREEN cycle.
- [ ] 2.2 Run the `GraphManagerTests` test selection and confirm the new unsupported-macro cases pass.
- [ ] 2.3 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full project after the change.
