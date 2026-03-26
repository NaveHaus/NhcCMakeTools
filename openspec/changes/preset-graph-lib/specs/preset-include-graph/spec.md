## ADDED Requirements

### Requirement: File Payload Management
The Include Graph SHALL store File payloads containing a file path and a list of pending (unresolved) include strings.

#### Scenario: Adding a file payload
- **WHEN** a File payload with path "CMakePresets.json" and pending include "${os}-presets.json" is added
- **THEN** it receives a Node ID and can be retrieved by that ID

### Requirement: Include Resolution
The Include Graph SHALL attempt to resolve all pending include strings on all file nodes using a provided Macro Context.

#### Scenario: Resolving includes with complete context
- **WHEN** the context provides "os=linux" and "linux-presets.json" is a known file
- **THEN** the pending include "${os}-presets.json" is resolved, an edge is created, and the pending include is removed

### Requirement: Structural State Computation
The Include Graph SHALL compute its state based on the resolution of include strings: Empty (no nodes), Resolved (all pending includes expanded and edges created), or Unresolved (some pending includes remain unexpandable).

#### Scenario: Computing Unresolved state
- **WHEN** a file node has a pending include "${arch}-presets.json" and the context lacks "arch"
- **THEN** the graph state is Unresolved
