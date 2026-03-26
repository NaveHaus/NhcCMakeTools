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

For each loaded file, the Manager SHALL extract the root-level `version` field and associate it with the file node.

If the file does not specify a root-level `version` field, the Manager SHALL mark the file node as Unresolved with reason `PresetVersionMissing` and SHALL NOT process `include` or preset arrays from that file.

If the file specifies `cmakeMinimumRequired`, the Manager SHALL associate it with the file node.

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

#### Scenario: File missing root version
- **WHEN** a file is loaded and parsed as JSON but has no root `version` field
- **THEN** the file node is marked Unresolved with reason `PresetVersionMissing`
- **AND** the Manager does not process includes or presets from that file

### Requirement: Simulated CMake Version
The Presets Graph Manager SHALL be configured with a simulated CMake version (major/minor/patch) used to validate preset file format constraints.

#### Scenario: Configuring a simulated CMake version
- **WHEN** a Presets Graph Manager is constructed with simulated CMake version 3.30.0
- **THEN** it uses that version for validation checks

### Requirement: Supported Preset File Version
The Presets Graph Manager SHALL determine the maximum supported preset file `version` based on the configured simulated CMake version.

At minimum, the following mappings SHALL be supported:
- CMake 3.19 supports preset file version 1
- CMake 3.20 supports preset file version 2
- CMake 3.21 supports preset file version 3
- CMake 3.23 supports preset file version 4
- CMake 3.24 supports preset file version 5
- CMake 3.25 supports preset file version 6
- CMake 3.27 supports preset file version 7
- CMake 3.28 supports preset file version 8
- CMake 3.30 supports preset file version 9
- CMake 3.31 supports preset file version 10
- CMake 4.3 supports preset file version 11

#### Scenario: Deriving supported preset file version
- **GIVEN** simulated CMake version 3.30.0
- **WHEN** the Manager computes the supported preset file version
- **THEN** it reports supported preset file version 9

### Requirement: File-Derived Macro Population
When resolving include strings within a specific preset file node, the Presets Graph Manager SHALL populate the Macro Context with macro values that can be derived from the file node.

At minimum, the manager SHALL provide `${fileDir}` as the directory containing the including preset file.

The manager SHALL provide `${dollar}` as a literal `$`.

The manager SHALL construct the Macro Context used for include expansion by:
- Starting from the caller-provided Macro Context macro map
- Injecting file-derived macros such as `${fileDir}`
- Injecting constant macros such as `${dollar}`

#### Scenario: Resolving an include using fileDir
- **GIVEN** a file node at path "./a/b/c/CMakePresets.json"
- **AND** that file node has an include string "${fileDir}/d/e/linux-presets.json"
- **WHEN** the Manager resolves include strings for that file node
- **THEN** the include expands to "./a/b/c/d/e/linux-presets.json"

### Requirement: Enforcing cmakeMinimumRequired
When a file node specifies `cmakeMinimumRequired`, the Manager SHALL compare it to the simulated CMake version configured on the Manager.

If the simulated version does not satisfy the file's `cmakeMinimumRequired`, the Manager SHALL mark the file node as Unresolved with reason `CMakeMinimumRequiredNotMet`.

#### Scenario: Simulated CMake too old
- **GIVEN** a file node declares cmakeMinimumRequired 3.30.0
- **AND** the Manager simulated CMake version is 3.23.0
- **WHEN** the Manager evaluates file constraints
- **THEN** the file node is marked Unresolved with reason `CMakeMinimumRequiredNotMet`

### Requirement: Enforcing Preset File Version
The Manager SHALL validate that each file node's preset file `version` is supported by the configured simulated CMake version.

If a file node's preset file `version` is not supported, the Manager SHALL mark the file node as Unresolved with reason `PresetVersionUnsupported`.

#### Scenario: Preset file version unsupported
- **GIVEN** the Manager simulated CMake version is 3.23.0
- **AND** a loaded file declares version 9
- **WHEN** the Manager validates the file
- **THEN** the file node is marked Unresolved with reason `PresetVersionUnsupported`

### Requirement: Enforcing Include Field Version
The Manager SHALL validate that the root-level `include` field is only used in preset files with `version` 4 or above.

If a file declares `version` less than 4 and also specifies a root-level `include` field, the Manager SHALL mark the file node as Unresolved with reason `IncludeFieldUnsupportedInPresetVersion`.

#### Scenario: Include field not allowed in older preset version
- **GIVEN** a loaded file declares version 3
- **AND** the file specifies a root-level `include` field
- **WHEN** the Manager validates the file
- **THEN** the file node is marked Unresolved with reason `IncludeFieldUnsupportedInPresetVersion`

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
