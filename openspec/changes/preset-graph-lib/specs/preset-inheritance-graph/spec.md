## ADDED Requirements

### Requirement: Preset Payload Management
The Inheritance Graph SHALL store Preset payloads containing a preset name, a hidden flag, an optional Condition AST, and a list of pending (unresolved) inheritance strings.

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

### Requirement: Condition Status Tracking
The Inheritance Graph SHALL evaluate the Condition AST of each Preset payload using a provided Macro Context.

The Inheritance Graph SHALL track a Preset availability status with at least:
- Active
- Hidden
- Disabled
- Unknown

The availability status SHALL be computed as follows:
- If the Preset is hidden, it is Hidden.
- Else, if the Preset uses `$vendor{...}` macros, it is Disabled.
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

### Requirement: Structural State Computation
The Inheritance Graph SHALL compute its state: Empty (no nodes), Resolved (all inherits resolved AND all Preset availabilities are definitively Active/Hidden/Disabled), or Unresolved (missing inherit targets OR one or more Presets are Unknown due to missing macros or environment values).

#### Scenario: Computing Unresolved state due to conditions
- **WHEN** all inheritance links are resolved but one Preset's condition depends on a missing macro
- **THEN** the Inheritance Graph state is Unresolved
