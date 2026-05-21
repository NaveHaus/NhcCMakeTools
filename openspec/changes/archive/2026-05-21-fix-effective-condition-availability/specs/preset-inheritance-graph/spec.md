## MODIFIED Requirements

### Requirement: Condition Status Tracking
The Inheritance Graph SHALL evaluate the effective condition state of each Preset payload using a provided Macro Context.

The effective condition payload used for availability evaluation SHALL already reflect standard `inherits` precedence for `condition`, including explicit `condition: null` clearing behavior for the current preset and non-propagation of explicit `null` to descendants.

The Inheritance Graph SHALL track a Preset availability status with at least:
- Active
- Hidden
- Disabled
- Unknown

The availability status SHALL be computed as follows:
- If the Preset is hidden, it is Hidden.
- Else, if the Preset uses `$vendor{...}` macros, it is Disabled.
- Else, if the effective condition is absent, it is Active.
- Else, if the condition evaluates to false, it is Disabled.
- Else, if the condition is indeterminate (unknown), it is Unknown.
- Else, it is Active.

#### Scenario: Evaluating an active preset condition
- **WHEN** a Preset's condition evaluates to true against the context and it is not hidden
- **THEN** the Inheritance Graph reports the Preset's availability as Active

#### Scenario: Hidden preset is not active
- **WHEN** a Preset is marked hidden
- **THEN** the Inheritance Graph reports the Preset's availability as Hidden

#### Scenario: Preset using vendor macro is disabled
- **WHEN** a Preset contains a string value using `$vendor{someMacro}`
- **THEN** the Inheritance Graph reports the Preset's availability as Disabled

#### Scenario: Preset with null condition has absent effective condition and is active
- **GIVEN** preset `P` defines `condition: null` and inherits no evaluable condition
- **WHEN** the Inheritance Graph evaluates preset availability
- **THEN** the effective condition for `P` is absent
- **AND** the Inheritance Graph reports preset `P` as Active

#### Scenario: Inherited false condition disables a child preset
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and does not define a local condition
- **WHEN** the Inheritance Graph evaluates preset availability
- **THEN** preset `C` uses the effective inherited condition from `P0`
- **AND** the Inheritance Graph reports preset `C` as Disabled

#### Scenario: Cycle diagnostics are preserved after effective-condition publishing
- **GIVEN** preset `A` inherits from [`B`]
- **AND** preset `B` inherits from [`A`]
- **AND** neither preset defines a local evaluable condition
- **WHEN** effective conditions are published and the Inheritance Graph evaluates availability
- **THEN** the Inheritance Graph reports presets `A` and `B` as Unresolved with reason `InheritanceCycle`
- **AND** the cycle-safe condition lookup does not suppress or replace the `InheritanceCycle` diagnostic
