## 1. Tighten the workflow preset typed API

- [ ] 1.1 Identify the remaining inherited `Preset` accessors that are still visible on `WorkflowPreset`, including condition-state helpers called out by the architecture review.
- [ ] 1.2 Update the `WorkflowPreset` typed API declarations so unsupported inherited accessors are hidden while workflow-supported accessors remain available.

## 2. Cover the API boundary with tests

- [ ] 2.1 Add or update tests under `tests/PresetsGraph` to verify `WorkflowPreset` exposes only workflow-supported typed accessors.
- [ ] 2.2 Add or update tests to verify inherited condition-state helpers are not available from the typed `WorkflowPreset` API.

## 3. Verify the change

- [ ] 3.1 Run the focused preset-model test target(s) needed for the TDD cycle after the API change.
- [ ] 3.2 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full build and test workflow before completing the change.
