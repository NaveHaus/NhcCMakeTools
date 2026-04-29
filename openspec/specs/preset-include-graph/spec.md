## Purpose

Define include graph behavior for CMake preset file relationships and include resolution diagnostics.

## Requirements

### Requirement: File Payload Management
The Include Graph SHALL store File payloads containing:
- A file path
- An optional preset file version
- An optional `cmakeMinimumRequired` version
- A list of pending (unresolved) include strings
- A resolution status

#### Scenario: Adding a file payload
- **WHEN** a File payload with path "CMakePresets.json" and version 10 and pending include "d/e/linux-presets.json" is added
- **THEN** it receives a Node ID and can be retrieved by that ID

### Requirement: Unresolved Reason Tracking
The Include Graph SHALL support marking a file node as Unresolved with an UnresolvedReason.
The UnresolvedReason enumeration SHALL include at least: `FileDoesNotExist`, `InvalidJson`, `MissingMacro`, `UnsupportedMacro`, `EnvironmentCycle`, `InvalidCondition`, `IncludeCycle`, `InheritanceCycle`, `CMakeMinimumRequiredNotMet`, `PresetVersionUnsupported`, `PresetVersionMissing`, and `IncludeFieldUnsupportedInPresetVersion`.

#### Scenario: Marking a file node as missing
- **WHEN** a file node is created for a file path that cannot be loaded
- **THEN** it is marked Unresolved with reason `FileDoesNotExist`

#### Scenario: Marking a file node as invalid
- **WHEN** a file node is created for a file path that is loaded but cannot be parsed as JSON
- **THEN** it is marked Unresolved with reason `InvalidJson`

### Requirement: Include Resolution
The Include Graph SHALL attempt to resolve all pending include strings on all file nodes using a provided Macro Context.
When an include string expands to a relative path, it SHALL be interpreted relative to the directory of the including file node.

#### Scenario: Resolving includes with complete context
- **WHEN** an include string "d/e/linux-presets.json" expands to a file path that exists in the graph
- **THEN** the pending include is resolved, an edge is created, and the pending include is removed

### Requirement: User Preset Root Inclusion
When a file node represents `CMakeUserPresets.json`, the Include Graph SHALL be able to represent the CMake-defined implicit include of a sibling `CMakePresets.json`.

If a sibling `CMakePresets.json` exists, the include relationship SHALL be directional from `CMakeUserPresets.json` to `CMakePresets.json`.

If no sibling `CMakePresets.json` exists, the Include Graph SHALL NOT synthesize that edge.

#### Scenario: User presets implicitly include project presets
- **GIVEN** a file node for "./a/b/CMakeUserPresets.json"
- **AND** a sibling file node exists at "./a/b/CMakePresets.json"
- **WHEN** the manager publishes the implicit user-root include
- **THEN** the Include Graph contains an edge from "./a/b/CMakeUserPresets.json" to "./a/b/CMakePresets.json"
- **AND** the reverse edge is not implied

### Requirement: Include Macro Policy
The Include Graph SHALL validate and expand include strings according to the preset file `version` of the including file node.

The following syntaxes SHALL be recognized in include strings:
- `${name}`
- `$penv{name}`

The following syntaxes SHALL be treated as Unsupported in include strings:
- `$env{name}`
- `$vendor{...}`

Preset-specific macros SHALL be treated as Unsupported in include strings.
At minimum, `${presetName}` and `${generator}` SHALL be treated as Unsupported.

#### Version 7 and 8 include rules
- Include strings SHALL support `$penv{}` macro expansion only.
- If an include string contains any `${...}` macro, the including file node SHALL be marked Unresolved with reason `UnsupportedMacro`.

#### Version 9+ include rules
- Include strings SHALL support `$penv{}` macro expansion.
- Include strings SHALL support `${fileDir}` and `${dollar}`.
- Include strings MAY support additional `${name}` macros if `name` is present in the Macro Context macro map provided for include expansion.

If an include string uses an Unsupported macro syntax or an Unsupported `${name}`, the Include Graph SHALL mark the including file node as Unresolved with reason `UnsupportedMacro`.

If an include string uses a Supported `${name}` that is not present in the Macro Context, the Include Graph SHALL leave it unexpanded and the including file node SHALL be marked Unresolved with reason `MissingMacro`.

#### Scenario: Include uses a disallowed macro
- **WHEN** a file node has an include string using `$env{HOME}`
- **THEN** the file node is marked Unresolved with reason `UnsupportedMacro`

#### Scenario: Include uses an unsupported preset-specific macro
- **WHEN** a file node has an include string using `${presetName}`
- **THEN** the file node is marked Unresolved with reason `UnsupportedMacro`

#### Scenario: Include uses a file-derived macro
- **WHEN** a file node at "./a/b/c/CMakePresets.json" has an include string "${fileDir}/d/e/linux-presets.json"
- **THEN** the include can expand to "./a/b/c/d/e/linux-presets.json" when `${fileDir}` is provided

#### Scenario: Include uses an unknown macro
- **WHEN** a file node has an include string "${unknown}/presets.json" and `unknown` is not present in the Macro Context
- **THEN** the include remains "${unknown}/presets.json"
- **AND** the file node is marked Unresolved with reason `MissingMacro`

### Requirement: Missing Include Files Are Retained
When an include string expands to a concrete file path but the file cannot be loaded, the Include Graph SHALL retain a File payload for that path and mark it as Unresolved with reason `FileDoesNotExist`.

#### Scenario: Include expands to a missing file
- **WHEN** a file node at "./a/b/c/CMakePresets.json" includes "d/e/linux-presets.json" and it resolves to "./a/b/c/d/e/linux-presets.json" and the file cannot be loaded
- **THEN** the Include Graph contains a file node for "./a/b/c/d/e/linux-presets.json"
- **AND** that file node is marked Unresolved with reason `FileDoesNotExist`

### Requirement: Structural State Computation
The Include Graph SHALL compute its state based on include expansion and file resolution: Empty (no nodes), Resolved (all pending includes expanded and all referenced files are resolved), or Unresolved (some pending includes remain unexpandable OR one or more referenced files are unresolved).

#### Scenario: Computing Unresolved state
- **WHEN** a file node has a pending include "${unknownMacro}/presets.json" and the macro cannot be expanded
- **THEN** the graph state is Unresolved
