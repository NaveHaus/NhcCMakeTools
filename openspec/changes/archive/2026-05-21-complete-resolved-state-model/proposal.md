## Why

The current resolved-state behavior in `preset-model` does not match the spec in three places: it still exposes a separate public `ResolvedPreset`, it records environment values without expanding them into preset-owned resolved state, and it only tracks a narrow subset of scalar fields as expandable resolved entries. This leaves callers without the per-field resolution status and field coverage that the spec already promises.

## What Changes

- Align resolved-state behavior with the `preset-model` spec by making preset-owned resolved fields the primary resolved-state contract.
- Require resolved environment entries to be expanded into preset-owned resolved state with per-entry resolution status preserved.
- Extend resolved scalar field handling beyond the current minimal set, starting with additional string-valued preset fields that are already relevant to CMake preset evaluation.
- Establish the migration direction away from the public `ResolvedPreset` API once preset-owned resolved state is sufficiently complete for existing callers.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `preset-model`: Change resolved-state requirements so preset-owned resolved fields, including expanded environment entries and broader scalar coverage, are the authoritative resolved-state interface.

## Impact

- OpenSpec delta for `preset-model`
- `PresetModel` resolved-state and caller migration design
- Follow-on implementation and tests for environment expansion, scalar coverage, and public API transition
