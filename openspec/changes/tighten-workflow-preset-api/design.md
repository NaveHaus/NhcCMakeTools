## Context

The current `preset-model` spec already states that `WorkflowPreset` exposes only the workflow-facing typed surface and does not expose unsupported fields inherited from `Preset`. The architecture review identified one remaining gap: condition-related helper accessors inherited from `Preset` are still visible on `WorkflowPreset`, which keeps the public typed API out of sync with the CMake workflow preset schema.

This change is intentionally narrow. It does not alter workflow parsing, storage, inheritance, or diagnostics behavior. It only tightens the typed API boundary so callers cannot observe unsupported workflow-only state through inherited helpers.

## Goals / Non-Goals

**Goals:**
- Make the `WorkflowPreset` typed API match the documented workflow preset contract.
- Explicitly cover inherited condition-state helpers as unsupported workflow accessors in the spec delta.
- Keep implementation work localized to API hiding and any directly affected tests.

**Non-Goals:**
- Changing workflow preset parsing or validation semantics
- Changing non-workflow preset APIs
- Introducing new workflow capabilities beyond hiding unsupported inherited accessors

## Decisions

1. Tighten the existing `preset-model` capability instead of introducing a new capability.
   - Rationale: this is a correction to the typed preset model contract, not a new behavior area.
   - Alternative considered: add a separate workflow-specific capability. Rejected because it would duplicate ownership of the same typed API contract.

2. Treat inherited condition-state helpers as part of the unsupported workflow surface.
   - Rationale: the workflow preset schema does not support `condition`, so helper APIs that expose the presence, absence, or explicit-null state of `condition` are also unsupported.
   - Alternative considered: leave helpers visible because they do not expose a workflow-specific data field directly. Rejected because they still let callers depend on invalid workflow condition state.

3. Prefer compile-time API hiding over runtime "do not use" conventions.
   - Rationale: the issue is an API-surface leak. The fix should remove typed access rather than relying on documentation or runtime behavior.
   - Alternative considered: document the helpers as unsupported while leaving them callable. Rejected because it preserves the contract mismatch.

## Risks / Trade-offs

- [Risk] Narrowing the typed API may break downstream code that incorrectly calls inherited workflow accessors. → Mitigation: keep the change scoped and document the exact contract in the spec delta so breakage is intentional and reviewable.
- [Risk] The spec may remain ambiguous about which inherited helpers are unsupported. → Mitigation: name condition-state helpers explicitly in the modified requirement and scenarios.
