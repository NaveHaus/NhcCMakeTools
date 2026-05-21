## Purpose

Define inheritance graph behavior for resolving preset inheritance, conditions, and availability.
## Requirements
### Requirement: Preset Payload Management
The Inheritance Graph SHALL store Preset payloads containing a preset name, a hidden flag, a parsed condition declaration, and a list of pending (unresolved) inheritance strings.

#### Scenario: Adding a preset payload
- **WHEN** a Preset payload with name "debug" and pending inherit "base" is added
- **THEN** it receives a Node ID and can be retrieved by that ID

### Requirement: Inheritance Resolution
The Inheritance Graph SHALL attempt to resolve all pending inheritance strings by matching them to the names of other Preset payloads in the graph.

#### Scenario: Resolving inheritance links
- **WHEN** a payload "debug" inherits "base", and a payload "base" exists in the graph
- **THEN** the pending inherit is removed and a directed edge is created from "debug" to "base"

### Requirement: Inheritance Cycle Detection
The Inheritance Graph SHALL detect cycles in preset inheritance links.

#### Scenario: Detecting a cyclic inherits relationship
- **WHEN** inheritance links create a cycle (e.g., A inherits B and B inherits A)
- **THEN** the Inheritance Graph marks affected presets as Unresolved with reason `InheritanceCycle`
- **AND** the Inheritance Graph state is Unresolved

### Requirement: Condition Inheritance Semantics
When the graph resolves inherited preset state, the `condition` field SHALL follow standard `inherits` precedence except for explicit `null`.

- If a preset does not define a local `condition` field, it SHALL inherit the first available inheritable condition from its parents according to the `inherits` list order.
- If a preset defines `condition: null`, that preset SHALL clear any inherited condition for itself.
- An explicit `condition: null` SHALL NOT be inherited by descendant presets.

#### Scenario: Null condition clears an inherited condition
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and defines `condition: null`
- **WHEN** the graph resolves inherited preset state
- **THEN** preset `C` has no effective evaluable condition
- **AND** preset `C` is not disabled by `P0`'s condition

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

### Requirement: Invalid Condition Diagnostics
If a Preset payload carries a condition-parse failure, the Inheritance Graph SHALL mark that preset Unresolved with reason `InvalidCondition`.

#### Scenario: Invalid condition object
- **WHEN** a Preset payload contains a condition-parse failure
- **THEN** that Preset is marked Unresolved with reason `InvalidCondition`
- **AND** the Inheritance Graph state is Unresolved

### Requirement: Structural State Computation
The Inheritance Graph SHALL compute its state: Empty (no nodes), Resolved (all inherits resolved AND all Preset availabilities are definitively Active/Hidden/Disabled), or Unresolved (missing inherit targets OR one or more Presets are Unknown due to missing macros or environment values OR one or more Presets have `InvalidCondition`).

#### Scenario: Computing Unresolved state due to conditions
- **WHEN** all inheritance links are resolved but one Preset's condition depends on a missing macro
- **THEN** the Inheritance Graph state is Unresolved

