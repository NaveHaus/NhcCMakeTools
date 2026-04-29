## Purpose

Define reusable directed graph primitives used by preset include and inheritance graphs.

## Requirements

### Requirement: Node Identification
The system SHALL use zero-based unique unsigned integers to identify nodes within a graph instance.

#### Scenario: Adding multiple nodes
- **WHEN** multiple payloads are added to a graph
- **THEN** each payload receives a unique sequential integer ID starting from 0

### Requirement: Edge Management
The system SHALL manage directed edges externally to the node payloads, represented as pairs of source and destination Node IDs.

#### Scenario: Querying node edges
- **WHEN** an edge is created from Node A to Node B
- **THEN** Node A's outgoing edges includes Node B
- **AND** Node B's incoming edges includes Node A

### Requirement: Directed Acyclic Property
The core graph structure SHALL provide mechanisms to verify the acyclic property of the directed graph.

#### Scenario: Cycle detection
- **WHEN** an edge is added that creates a cycle (e.g., A -> B -> C -> A)
- **THEN** the cycle detection mechanism reports that the graph contains a cycle
