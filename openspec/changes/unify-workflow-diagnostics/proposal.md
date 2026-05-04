## Why

Workflow validation currently produces manager-owned diagnostics that are separate from the workflow preset's own unresolved state. That forces consumers to query two places to understand whether a workflow preset is structurally usable, and it leaves the contract around workflow failure surfacing under-specified.

## What Changes

- Mark structurally invalid workflow presets as unresolved at the preset level while preserving step-level workflow diagnostics for detail.
- Define a dedicated workflow-validation unresolved reason so callers can distinguish workflow-structure failures from other preset failures.
- Clarify that workflow presets still remain outside inheritance-graph participation even when workflow validation fails.
- Tighten the manager contract so workflow validation continues to be non-fatal to the overall refresh loop while publishing both preset-level and diagnostic-level workflow failure information.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `preset-model`: define how workflow validation failure is represented on the workflow preset itself, including the preset-level unresolved reason and retained diagnostics.
- `preset-graph-manager`: define manager behavior for marking invalid workflow presets unresolved while keeping them queryable and out of inheritance resolution.

## Impact

- Affected specs: `preset-model`, `preset-graph-manager`
- Affected API surface: workflow preset resolved-state and workflow-diagnostic query behavior
- Affected code area: preset-model workflow state, manager workflow validation flow, and any queries that consume workflow preset unresolved state plus diagnostics
