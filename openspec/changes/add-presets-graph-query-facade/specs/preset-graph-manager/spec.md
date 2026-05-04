## ADDED Requirements

### Requirement: Manager-Provided Query Facade
The Presets Graph Manager SHALL expose the preset-graph query facade as the preferred high-level read-only query entry point for its current applied state.

The manager SHALL keep direct access to lower-level graph and model APIs available for advanced consumers.

The manager SHALL ensure the facade reflects the manager-owned state produced by the most recent successful `ApplyContext(...)` evaluation.

#### Scenario: Reading the current graph state through the manager
- **WHEN** a consumer queries the manager after `ApplyContext(...)`
- **THEN** the manager provides a high-level read-only query facade for the current applied state
- **AND** direct lower-level graph and model APIs remain available for advanced use cases
