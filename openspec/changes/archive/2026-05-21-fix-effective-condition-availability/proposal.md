## Why

Inherited preset conditions are specified behavior, but the current graph refresh path only publishes a preset's direct condition into the inheritance graph. That leaves descendant presets incorrectly reported as active when they should inherit a disabling condition, and it blocks reuse of `PresetModel::ResolveCondition()` because that resolver is not safe against cyclic inherits chains during refresh.

## What Changes

- Require inheritance-graph availability evaluation to use each preset's effective inherited condition, not only its direct local condition.
- Require preset-model condition resolution to guard against cyclic inherits traversal so callers can safely resolve an effective condition before graph cycle diagnostics are finalized.
- Define the resulting behavior for explicit `condition: null` and cyclic inherits cases during effective-condition lookup.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `preset-inheritance-graph`: Tighten availability evaluation requirements so the graph uses the effective inherited condition payload when determining preset availability.
- `preset-model`: Tighten condition-resolution requirements so effective-condition lookup is cycle-safe and preserves the non-inheritable behavior of explicit `condition: null`.

## Impact

- OpenSpec deltas for `preset-inheritance-graph` and `preset-model`
- `PresetModel::ResolveCondition()` traversal behavior
- Inheritance-graph refresh logic that publishes condition payloads for availability evaluation
- Follow-on tests for inherited disablement, explicit null breaks, and cyclic inherits handling
