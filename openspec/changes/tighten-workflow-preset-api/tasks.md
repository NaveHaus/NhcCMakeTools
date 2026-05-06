## 1. RED — write failing compile-time API-visibility tests

- [ ] 1.1 In `tests/PresetsGraph`, add static-assert or compile-error tests that verify `GetConditionState`, `SetConditionExplicitNull`, and `ClearCondition` are NOT accessible on a `WorkflowPreset` instance. These tests MUST fail to compile before the implementation change.
- [ ] 1.2 Confirm the tests fail as expected (compilation error or test build failure) to establish the RED state.

## 2. GREEN — hide the remaining inherited accessors

- [ ] 2.1 Identify all remaining inherited `Preset` accessors that are still visible on `WorkflowPreset`: at minimum `GetConditionState`, `SetConditionExplicitNull`, and `ClearCondition`.
- [ ] 2.2 Add `private` `using` declarations in `WorkflowPreset` for each accessor identified in 2.1 so they are compile-time inaccessible to callers.
- [ ] 2.3 Confirm the RED tests from step 1 now pass (compile and link without error) and that the existing preset-model test target still passes.

## 3. REFACTOR — verify and clean up

- [ ] 3.1 Review `WorkflowPreset` for any remaining inherited public accessors that are inconsistent with the `name`-and-`steps`-only typed contract; hide any that are found.
- [ ] 3.2 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full build and test workflow is green before completing the change.
