## 1. Manager include-policy delegation

- [x] 1.1 **RED**: Add a failing manager-level test in `tests/PresetsGraph/GraphManagerTests.cpp` verifying that an include using `$env{HOME}` is reported as `UnsupportedMacro` when resolved through `ApplyContext()`.
- [x] 1.2 **RED**: Add a failing manager-level test verifying that preset-specific include macros such as `${presetName}` remain `UnsupportedMacro` through the manager path.
- [x] 1.3 **RED**: Add a failing manager-level test verifying that a version 7 or 8 file using `${fileDir}` in `include` is reported as `UnsupportedMacro` through the manager path.
- [x] 1.4 **GREEN**: Refactor `PresetsGraph::ApplyContext()` so per-file include resolution delegates macro-policy enforcement to `PresetIncludeGraph` while preserving file-derived macro injection and iterative discovery.
- [x] 1.5 **GREEN**: Update manager/include-graph integration plumbing so newly resolved file paths continue to load and refresh correctly after delegation.
- [x] 1.6 **REFACTOR**: Review the manager/include-graph boundary for duplicated include-policy logic and remove any remaining redundant checks.
- [x] 1.7 **REFACTOR**: Verify that the delegation refactor does not regress non-macro include semantics: (a) relative-path resolution is internal to `PresetIncludeGraph` and tested in `GraphIncludeTests`; (b) include-cycle detection operates in the manager's iterative loop, is not in the delegation path, and is covered by the permanent manager spec's Resolution Cycle Detection requirement; (c) repeated inclusion tolerance (same file included via multiple paths is loaded once, not marked `IncludeCycle`) is covered by the "Applying context tolerates repeated inclusion" scenario in the delta spec — add a manager-boundary test for it if no existing coverage is found.

## 2. Verification

- [x] 2.1 Build the affected manager test target after the RED/GREEN cycle.
- [x] 2.2 Run the `GraphManagerTests` test selection and confirm the new unsupported-macro cases pass.
- [x] 2.3 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full project after the change.
