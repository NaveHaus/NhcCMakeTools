## ADDED Requirements

### Requirement: Graph Orchestration
The Presets Graph Manager SHALL own instances of the Include Graph and the Inheritance Graph.

#### Scenario: Manager initialization
- **WHEN** a Presets Graph Manager is instantiated
- **THEN** it contains empty, independent Include and Inheritance graphs

### Requirement: Context Application Loop
The Presets Graph Manager SHALL orchestrate the resolution loop when a new Macro Context is applied, first resolving the Include graph, handling any newly discovered include file paths, and subsequently resolving the Inheritance graph.

#### Scenario: Applying context discovers new files
- **WHEN** a context is applied and an include string expands to a new file path "extra-presets.json"
- **THEN** the Manager orchestrates loading the new file, adding its nodes, and re-running resolution until no new files are found

### Requirement: File Loading and JSON Parsing
The Presets Graph Manager SHALL be constructed with a file loader abstraction.
When a newly discovered include file path is not yet represented in the Include Graph, the Manager SHALL use the file loader to read the file content as a string and parse it using `nlohmann::json::parse(...)`.

#### Scenario: Loading and parsing a newly discovered include file
- **WHEN** applying a context discovers an include file path that is not yet loaded
- **THEN** the Manager loads the file content using the file loader
- **AND** parses the content as JSON using `nlohmann::json::parse(...)`

#### Scenario: Handling a file loader failure
- **WHEN** the file loader reports a failure while loading a newly discovered include file
- **THEN** the Manager adds or updates the corresponding file node and marks it Unresolved with reason `FileDoesNotExist`
- **AND** the Manager continues resolving other independent include paths
- **AND** the Manager marks the overall state as Unresolved

#### Scenario: Handling invalid JSON
- **WHEN** a newly discovered include file is loaded but cannot be parsed as JSON
- **THEN** the Manager adds or updates the corresponding file node and marks it Unresolved with reason `InvalidJson`
- **AND** the Manager continues resolving other independent include paths
- **AND** the Manager marks the overall state as Unresolved

### Requirement: Resolution Cycle Detection
The Presets Graph Manager SHALL implement strict cycle detection during the iterative resolution loop to prevent infinite recursive file discovery.

#### Scenario: Detecting cyclic includes during resolution
- **WHEN** applying a context resolves an include path to a file that was already loaded in the current resolution chain
- **THEN** the Manager marks the affected file nodes as Unresolved with reason `IncludeCycle`
- **AND** the Manager marks the overall state as Unresolved

### Requirement: Composite State Computation
The Presets Graph Manager SHALL compute its overall composite state based strictly on the states of its underlying Include and Inheritance graphs.

#### Scenario: Computing composite Unresolved state
- **WHEN** the Include graph is Resolved but the Inheritance graph is Unresolved
- **THEN** the Presets Graph Manager reports an overall state of Unresolved
