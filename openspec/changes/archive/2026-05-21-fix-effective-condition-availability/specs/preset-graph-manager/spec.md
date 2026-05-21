## ADDED Requirements

### Requirement: Effective-Condition Payload Publishing
After ingesting preset collections and before invoking availability evaluation in the Inheritance Graph, the Presets Graph Manager SHALL publish the effective condition for each Configure, Build, Test, and Package preset payload.

For each such preset, the Manager SHALL obtain the effective condition by calling `PresetModel::ResolveCondition()` and store the result in the corresponding Inheritance Graph payload.

If `ResolveCondition()` returns no effective condition (absent), the Manager SHALL leave the Inheritance Graph payload's effective condition unset. The Inheritance Graph SHALL treat an absent effective condition as Active in the absence of other disabling conditions.

Workflow presets do not participate in this effective-condition publishing step.

#### Scenario: Inherited condition is published before availability evaluation
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and does not define a local condition
- **WHEN** the Manager publishes effective conditions after preset collection ingestion
- **THEN** the Inheritance Graph payload for preset `C` carries the effective condition resolved from `P0`
- **AND** the Inheritance Graph reports preset `C` as Disabled during availability evaluation

#### Scenario: Workflow presets are excluded from effective-condition publishing
- **GIVEN** a workflow preset `W` references configure preset `C` as a step
- **WHEN** the Manager publishes effective conditions during refresh
- **THEN** preset `W` is not passed to `PresetModel::ResolveCondition()` for effective-condition publishing
