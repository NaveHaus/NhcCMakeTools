## ADDED Requirements

### Requirement: Preset Type Identification
The system SHALL represent CMake presets as one of the following types:
- ConfigurePreset
- BuildPreset
- TestPreset
- PackagePreset
- WorkflowPreset

All preset types SHALL be derived from a common base Preset concept.

#### Scenario: Identifying a preset type
- **WHEN** a preset originates from the `buildPresets` array in a preset file
- **THEN** it is represented as a BuildPreset

### Requirement: Raw And Expanded Views
For each preset, the system SHALL retain:
- The raw/original JSON object as read from disk
- An expanded view for macro-expandable string fields used by the library

The expanded view SHALL preserve the original strings alongside any computed expansions.

#### Scenario: Preserving raw and expanded values
- **WHEN** a preset contains an environment value "$env{VCPKG_ROOT}/scripts"
- **THEN** the raw value is retained
- **AND** the system can provide an expanded view of the value given a Macro Context

### Requirement: Preset-Associated Macro Population
When expanding a macro-expandable field that belongs to a specific preset, the system SHALL populate the Macro Context with preset-associated macro values for that preset.

At minimum, the system SHALL provide `${presetName}` with the name of the preset currently being expanded.

#### Scenario: Providing presetName during expansion
- **GIVEN** a preset named "default"
- **WHEN** a field belonging to that preset is expanded
- **THEN** `${presetName}` expands to "default"

### Requirement: Expansion Uses Active Preset Context
All macro expansion for a field belonging to preset C SHALL be evaluated using preset-associated macro values for preset C, even when the field value originates from inheritance.

#### Scenario: Inherited value expands using the child presetName
- **GIVEN** a parent preset P0 with environment {"X": "${presetName}"}
- **AND** a child preset C named "child" inherits from [P0]
- **WHEN** C's environment is expanded
- **THEN** C's expanded environment value for "X" is "child"

### Requirement: Preset-Associated Generator Macro
When expanding a macro-expandable field that belongs to a specific preset, the system SHALL provide `${generator}` when the effective generator for that preset is known.

For a ConfigurePreset, the effective generator is the resolved value of its `generator` field after inheritance.
For a BuildPreset, TestPreset, or PackagePreset, `${generator}` SHALL resolve to the effective generator of the associated ConfigurePreset referenced by `configurePreset`.

If the effective generator is not known, `${generator}` SHALL remain unresolved.

#### Scenario: Configure preset provides generator macro
- **GIVEN** a ConfigurePreset named "cfg" with generator "Ninja"
- **WHEN** a field belonging to that preset is expanded
- **THEN** `${generator}` expands to "Ninja"

#### Scenario: Build preset derives generator from configurePreset
- **GIVEN** a ConfigurePreset named "cfg" with generator "Ninja"
- **AND** a BuildPreset named "bld" with configurePreset "cfg"
- **WHEN** a field belonging to BuildPreset "bld" is expanded
- **THEN** `${generator}` expands to "Ninja"

### Requirement: Preset-Specific Macro Support
The system SHALL support all macros documented as "preset-specific" in the CMake presets specification.

At minimum, the system SHALL support:
- `${presetName}`
- `${generator}`

#### Scenario: Expanding all supported preset-specific macros
- **GIVEN** a ConfigurePreset named "cfg" with generator "Ninja"
- **WHEN** a string "${presetName}-${generator}" is expanded for that preset
- **THEN** the result is "cfg-Ninja"

### Requirement: Inheritance Precedence
When a preset inherits from multiple parent presets, and two parents provide conflicting values for the same scalar field, the earlier preset in the `inherits` list SHALL take precedence.

#### Scenario: Earlier inherits entry wins scalar conflict
- **GIVEN** preset C inherits from [P0, P1]
- **AND** both P0 and P1 provide an `installDir`
- **WHEN** C is resolved
- **THEN** C uses P0's `installDir` value

### Requirement: Environment Merge Semantics
The `environment` map SHALL be merged using these rules:
- A preset's environment is the union of its own `environment` entries and all inherited `environment` entries.
- If multiple parents define the same environment key, the earlier preset in the `inherits` list SHALL take precedence.
- Setting an environment key to `null` SHALL remove that key from the resulting environment, even if it was inherited.

For BuildPreset, TestPreset, and PackagePreset, when `inheritConfigureEnvironment` is true, the associated ConfigurePreset environment SHALL be merged after all inherited Build/Test/Package environments but before environment variables explicitly specified in the Build/Test/Package preset.

#### Scenario: Earlier inherits entry wins environment key conflict
- **GIVEN** preset C inherits from [P0, P1]
- **AND** P0 defines environment {"X": "0"}
- **AND** P1 defines environment {"X": "1"}
- **WHEN** C is resolved
- **THEN** C's environment contains {"X": "0"}

#### Scenario: Null removes inherited environment key
- **GIVEN** preset C inherits from [P0]
- **AND** P0 defines environment {"X": "0"}
- **AND** C defines environment {"X": null}
- **WHEN** C is resolved
- **THEN** C's environment does not contain key "X"

#### Scenario: Build preset merges configure environment in the middle
- **GIVEN** a ConfigurePreset "cfg" with environment {"X": "cfg"}
- **AND** a BuildPreset "bld" inherits from [P0]
- **AND** P0 defines environment {"X": "p0", "Y": "p0"}
- **AND** BuildPreset "bld" sets configurePreset "cfg" and inheritConfigureEnvironment true
- **AND** BuildPreset "bld" defines environment {"Y": "bld"}
- **WHEN** BuildPreset "bld" is resolved
- **THEN** the resulting environment contains {"X": "cfg", "Y": "bld"}

### Requirement: Environment Value Expansion With Cycle Detection
The system SHALL support expanding environment map values that reference other environment variables using `$env{}` and `$penv{}`.

Environment values MAY reference each other and MAY be listed in any order.
The system SHALL detect reference cycles among environment keys.

If an environment value references a missing variable, the reference SHALL remain unresolved (not substituted with an empty string).

#### Scenario: Resolving environment variables out of order
- **GIVEN** a preset environment {"A": "$env{B}", "B": "b"}
- **WHEN** the environment is expanded
- **THEN** "A" expands to "b"

#### Scenario: Detecting an environment cycle
- **GIVEN** a preset environment {"A": "$env{B}", "B": "$env{A}"}
- **WHEN** the environment is expanded
- **THEN** the system marks the preset as Unresolved with reason `EnvironmentCycle`

#### Scenario: Missing environment variable remains unresolved
- **GIVEN** a preset environment {"A": "$env{DOES_NOT_EXIST}"}
- **WHEN** the environment is expanded
- **THEN** "A" remains "$env{DOES_NOT_EXIST}"
- **AND** the system can indicate the expansion was partial

### Requirement: Library-Relevant Expanded Fields
The system SHALL provide a minimal typed view of each preset sufficient to build and evaluate the graphs, including:
- `name`
- `inherits`
- `condition`
- `environment`

Additionally:
- BuildPreset, TestPreset, and PackagePreset SHALL expose `configurePreset`.
- BuildPreset, TestPreset, and PackagePreset SHALL expose `inheritConfigureEnvironment`.
- ConfigurePreset SHALL expose `generator`.

All other fields MAY be retained only in the raw/original JSON for this change.

#### Scenario: Minimal typed access for graph resolution
- **WHEN** a preset is added to the system
- **THEN** its name, inherits list, condition, and environment can be queried without requiring consumers to inspect raw JSON
