## MODIFIED Requirements

### Requirement: Context Application Loop
The Presets Graph Manager SHALL orchestrate the resolution loop when a new Macro Context is applied, first resolving the Include graph, handling any newly discovered include file paths, refreshing typed preset collections from successfully loaded files, and subsequently resolving the Inheritance graph.

The manager SHALL use the Include Graph as the authoritative resolver for include-string expansion and macro-policy enforcement for each file node it processes.

#### Scenario: Applying context discovers new files
- **WHEN** a context is applied and an include string expands to a new file path "extra-presets.json"
- **THEN** the Manager orchestrates loading the new file, adding its nodes, and re-running resolution until no new files are found

#### Scenario: Applying context surfaces unsupported include macros from the include graph
- **GIVEN** a loaded preset file contains an `include` string using an unsupported macro syntax
- **WHEN** the Manager applies a context and resolves includes for that file
- **THEN** the Manager preserves the Include Graph unresolved diagnostic for that file node
- **AND** the unresolved reason reported for that file is `UnsupportedMacro`

#### Scenario: Applying context tolerates repeated inclusion of the same file
- **GIVEN** file A includes file B and file C
- **AND** file C also includes file B
- **WHEN** the Manager applies a context
- **THEN** file B is loaded exactly once
- **AND** file B is not marked Unresolved with reason `IncludeCycle`

### Requirement: File-Derived Macro Population
When resolving include strings within a specific preset file node, the Presets Graph Manager SHALL populate the Macro Context with macro values that can be derived from the file node.

At minimum, the manager SHALL provide `${fileDir}` as the directory containing the including preset file.

The manager SHALL provide `${dollar}` as a literal `$`.

The manager SHALL construct the Macro Context used for include expansion by:
- Starting from the caller-provided Macro Context macro map
- Injecting file-derived macros such as `${fileDir}`
- Injecting constant macros such as `${dollar}`

The manager SHALL provide that per-file Macro Context to the Include Graph resolution step rather than applying separate include-policy logic in the manager.

#### Scenario: Resolving an include using fileDir
- **GIVEN** a file node at path "./a/b/c/CMakePresets.json"
- **AND** that file node has an include string "${fileDir}/d/e/linux-presets.json"
- **WHEN** the Manager resolves include strings for that file node
- **THEN** the include expands to "./a/b/c/d/e/linux-presets.json"
