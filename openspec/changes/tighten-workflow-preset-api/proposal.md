## Why

`WorkflowPreset` still exposes typed accessors inherited from `Preset` for fields that the CMake workflow preset schema does not support. That keeps the public typed API broader than the documented model contract and leaves callers able to depend on invalid workflow-only state.

## What Changes

- Tighten the typed `WorkflowPreset` API so unsupported inherited accessors are hidden from callers.
- Preserve the existing workflow-preset model contract that only `name`, `steps`, `displayName`, `description`, and `vendor` are part of the workflow-facing typed surface.
- Clarify the spec requirements around unsupported workflow accessors so implementation and tests target the remaining API-hiding gap directly.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `preset-model`: Narrow the `WorkflowPreset` typed API to hide all unsupported inherited accessors that remain visible from `Preset`.

## Impact

- Affected spec: `preset-model`
- Affected API surface: `WorkflowPreset` typed accessors in the preset model
- Affected code area: `src/PresetsGraph/PresetModel.h` and related tests for workflow preset API visibility
