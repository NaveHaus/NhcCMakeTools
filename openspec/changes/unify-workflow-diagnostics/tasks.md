## 1. Workflow Preset Unresolved State (RED -> GREEN -> REFACTOR)

- [ ] 1.1 RED: Add a failing test under `tests/PresetsGraph` that validates a workflow preset with a mismatched later step and expects the workflow preset to become unresolved with reason `InvalidWorkflowSteps` while remaining present in `PresetModel`.
- [ ] 1.2 GREEN: Implement the minimal workflow-validation update so invalid workflow structure marks the workflow preset unresolved with reason `InvalidWorkflowSteps`.
- [ ] 1.3 REFACTOR: Isolate any shared workflow-validation state update logic so preset-level unresolved marking and workflow-diagnostic emission stay consistent.

## 2. Unified Workflow Diagnostics Surface (RED -> GREEN -> REFACTOR)

- [ ] 2.1 RED: Add a failing test under `tests/PresetsGraph` that validates an invalid workflow preset still retains step-level workflow diagnostics after being marked unresolved.
- [ ] 2.2 GREEN: Implement the minimal manager and model behavior needed to preserve detailed workflow diagnostics alongside the preset-level unresolved state.
- [ ] 2.3 RED: Add a failing test under `tests/PresetsGraph` that validates invalid workflow presets remain out of the Inheritance Graph even when workflow validation fails.
- [ ] 2.4 GREEN: Implement the minimal workflow-validation path needed to keep invalid workflow presets queryable without introducing inheritance-graph participation.
- [ ] 2.5 REFACTOR: Review workflow validation naming and helper boundaries so the unified workflow-state contract is explicit and does not leak inheritance-oriented assumptions.

## 3. Verification (RED -> GREEN -> REFACTOR)

- [ ] 3.1 RED: Build and run the focused preset-graph test target covering the new workflow-validation scenarios and confirm the new expectations fail for the intended reasons before the implementation is complete.
- [ ] 3.2 GREEN: Re-run the focused preset-graph test target after implementation and confirm the new workflow unresolved-state and diagnostic scenarios pass.
- [ ] 3.3 GREEN: Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full configure-build-test workflow after the change.
- [ ] 3.4 REFACTOR: Review the new and updated tests for overlap so each scenario stays behavior-focused and aligned with the spec contract.
