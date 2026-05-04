## ADDED Requirements

### Requirement: Unified Read-Only Query Facade
The system SHALL provide a read-only preset-graph query facade that serves as a single high-level entry point for common post-resolution queries.

The facade SHALL expose queries across the manager's current include-graph, inheritance-graph, and preset-model state without requiring callers to coordinate those subsystems directly.

The facade SHALL NOT apply macro context, resolve includes, resolve inheritance, or recompute resolved preset state.

#### Scenario: Common consumer uses one query entry point
- **WHEN** a consumer needs preset-graph information after context has been applied
- **THEN** the consumer can use one facade entry point for common read-only queries
- **AND** the consumer does not need to manually coordinate the include graph, inheritance graph, and preset model APIs

### Requirement: Preset Summary Queries
The facade SHALL support querying typed preset summaries for presets currently known to the manager.

For each queried preset, the facade SHALL expose:
- the preset name
- the concrete preset type
- the source preset file path
- the effective availability
- any preset-level unresolved reason
- any workflow validation diagnostics associated with that preset

#### Scenario: Retrieving a typed preset summary
- **WHEN** a consumer queries the facade for a known preset
- **THEN** the facade returns that preset's typed summary
- **AND** the summary includes the preset's source file path and effective availability

### Requirement: Resolved Field Queries Reuse Model State
The facade SHALL expose a preset's current resolved field state exactly as currently stored on the typed preset in the preset model.

The facade SHALL preserve the resolved-state distinction between unresolved, partially resolved, and fully resolved fields.

The facade SHALL NOT synthesize a separate facade-owned resolved-state snapshot.

#### Scenario: Reading resolved field state through the facade
- **GIVEN** a preset has current resolved field entries in the preset model
- **WHEN** a consumer queries the facade for that preset's resolved fields
- **THEN** the facade returns the current resolved field state from the preset model
- **AND** each field retains its existing resolution status

### Requirement: Relationship Queries
The facade SHALL support common relationship lookups across the current manager-owned graph state.

At minimum, the facade SHALL support:
- file-to-included-files queries
- file-to-declaring-presets queries
- preset-to-parent-preset queries
- preset-to-child-preset queries

#### Scenario: Inspecting preset inheritance relationships
- **WHEN** a consumer queries the facade for a preset's parent or child presets
- **THEN** the facade returns the relationships derived from the current inheritance graph state

#### Scenario: Inspecting file-level preset ownership
- **WHEN** a consumer queries the facade for the presets declared by a loaded file
- **THEN** the facade returns the presets currently originating from that file
