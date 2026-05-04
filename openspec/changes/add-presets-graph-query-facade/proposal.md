## Why

Common consumers of `NhcPresetGraph` need answers about files, presets, availability, and resolved state without manually coordinating the include graph, inheritance graph, and preset model APIs. That coordination is currently left to each caller, which duplicates library-specific knowledge and makes it harder to adopt the corrected include, inherited-condition, and resolved-state behavior consistently.

## What Changes

- Add a thin query facade API that exposes common read-only preset-graph queries through one entry point.
- Define facade queries in terms of the existing manager, include graph, inheritance graph, and preset model rather than introducing a second resolution pipeline.
- Standardize how callers retrieve effective preset availability, resolved field state, workflow diagnostics, and file/preset relationship lookups after `ApplyContext(...)`.
- Preserve existing lower-level graph and model APIs for advanced consumers that need direct access.

## Capabilities

### New Capabilities
- `preset-graph-query-facade`: A read-only facade for common post-resolution queries across preset files, presets, effective availability, resolved fields, and diagnostics.

### Modified Capabilities
- `preset-graph-manager`: Expose the new facade from the manager-level API surface as the preferred high-level query entry point after context application.

## Impact

- **Code**: Adds a small query-oriented API layer in the preset-graph library and integrates it with the manager surface.
- **APIs**: Introduces a new high-level read-only facade while keeping direct graph/model access available.
- **Dependencies**: No new external dependencies are expected.
