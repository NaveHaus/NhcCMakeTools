## ADDED Requirements

### Requirement: Preset Payload Management
The Inheritance Graph SHALL store Preset payloads containing a preset name, an optional Condition AST, and a list of pending (unresolved) inheritance strings.

#### Scenario: Adding a preset payload
- **WHEN** a Preset payload with name "debug" and pending inherit "base" is added
- **THEN** it receives a Node ID and can be retrieved by that ID

### Requirement: Inheritance Resolution
The Inheritance Graph SHALL attempt to resolve all pending inheritance strings by matching them to the names of other Preset payloads in the graph.

#### Scenario: Resolving inheritance links
- **WHEN** a payload "debug" inherits "base", and a payload "base" exists in the graph
- **THEN** the pending inherit is removed and a directed edge is created from "debug" to "base"

### Requirement: Condition Status Tracking
The Inheritance Graph SHALL evaluate the Condition AST of each Preset payload using a provided Macro Context and track the resulting status (Enabled, Disabled, or Unknown).

#### Scenario: Evaluating an enabled condition
- **WHEN** a Preset's condition evaluates to true against the context
- **THEN** the Inheritance Graph reports the Preset's status as Enabled

### Requirement: Structural State Computation
The Inheritance Graph SHALL compute its state: Empty (no nodes), Resolved (all inherits resolved AND all conditions evaluate to definitively true/false), or Unresolved (missing inherit targets OR conditions return unknown due to missing macros).

#### Scenario: Computing Unresolved state due to conditions
- **WHEN** all inheritance links are resolved but one Preset's condition depends on a missing macro
- **THEN** the Inheritance Graph state is Unresolved
