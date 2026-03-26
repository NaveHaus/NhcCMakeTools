## ADDED Requirements

### Requirement: Graph Orchestration
The Presets Graph Manager SHALL own instances of the Include Graph and the Inheritance Graph.

#### Scenario: Manager initialization
- **WHEN** a Presets Graph Manager is instantiated
- **THEN** it contains empty, independent Include and Inheritance graphs

### Requirement: Context Application Loop
The Presets Graph Manager SHALL orchestrate the resolution loop when a new Macro Context is applied, first resolving the Include graph, handling any newly discovered files, and subsequently resolving the Inheritance graph.

#### Scenario: Applying context discovers new files
- **WHEN** context "os=windows" is applied and the Include graph discovers "windows-presets.json"
- **THEN** the Manager orchestrates loading the new file, adding its nodes, and re-running resolution until no new files are found

### Requirement: Resolution Cycle Detection
The Presets Graph Manager SHALL implement strict cycle detection during the iterative resolution loop to prevent infinite recursive file discovery.

#### Scenario: Detecting cyclic includes during resolution
- **WHEN** applying a context resolves an include path to a file that was already loaded in the current resolution chain
- **THEN** the Manager immediately halts further resolution
- **AND** the Manager reports a structural error or marks the overall state as Unresolved

### Requirement: Composite State Computation
The Presets Graph Manager SHALL compute its overall composite state based strictly on the states of its underlying Include and Inheritance graphs.

#### Scenario: Computing composite Unresolved state
- **WHEN** the Include graph is Resolved but the Inheritance graph is Unresolved
- **THEN** the Presets Graph Manager reports an overall state of Unresolved
