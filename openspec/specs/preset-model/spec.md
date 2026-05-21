## Purpose

Define the typed CMake preset model, resolved state, inheritance merge behavior, and workflow compatibility diagnostics.
## Requirements
### Requirement: Preset Type Identification
The system SHALL represent CMake presets as one of the following types:
- ConfigurePreset
- BuildPreset
- TestPreset
- PackagePreset
- WorkflowPreset

All preset types SHALL be derived from a common base Preset class that defines fields shared across preset types.

#### Scenario: Identifying a preset type
- **WHEN** a preset originates from the `buildPresets` array in a preset file
- **THEN** it is represented as a BuildPreset

### Requirement: Preset Type Hierarchy
The system SHALL implement a class hierarchy for preset types:
- A base `Preset` class SHALL define fields common to Configure, Build, Test, and Package presets: `name`, `hidden`, `inherits`, parsed `condition` declaration, `environment`.
- `ConfigurePreset` SHALL derive from `Preset` and add: `generator`, `installDir`.
- `BuildPreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `TestPreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `PackagePreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `WorkflowPreset` MAY derive from `Preset` for implementation convenience, but the typed API SHALL expose only `name` and `steps` for workflow presets. Per the CMake specification, workflow presets do not support `hidden`, `inherits`, `condition`, or `environment`, so callers SHALL NOT be given typed accessors for those fields on a `WorkflowPreset`. In addition, callers SHALL NOT be given typed accessors for inherited condition-state helpers (`GetConditionState`, `SetConditionExplicitNull`, `ClearCondition`) that allow observing or modifying workflow condition state.

The `PresetModel` SHALL store presets polymorphically and provide type-safe accessors.

#### Scenario: Accessing type-specific fields on ConfigurePreset
- **GIVEN** a ConfigurePreset named "cfg" with generator "Ninja"
- **WHEN** the preset is retrieved as a ConfigurePreset
- **THEN** the `generator` field is accessible and equals "Ninja"

#### Scenario: Accessing type-specific fields on BuildPreset
- **GIVEN** a BuildPreset named "bld" with configurePreset "cfg"
- **WHEN** the preset is retrieved as a BuildPreset
- **THEN** the `configurePreset` field is accessible and equals "cfg"
- **AND** the `inheritConfigureEnvironment` field is accessible

#### Scenario: WorkflowPreset omits unsupported typed accessors
- **GIVEN** a WorkflowPreset named "wf" with steps
- **WHEN** the preset is queried
- **THEN** its typed API does NOT expose inherited `hidden`, `inherits`, `condition`, or `environment` accessors
- **AND** it exposes `name` and `steps`

#### Scenario: WorkflowPreset omits inherited condition-state helpers
- **GIVEN** a WorkflowPreset named "wf"
- **WHEN** the preset is queried through the typed workflow API
- **THEN** callers cannot call `GetConditionState()` on a `WorkflowPreset`
- **AND** callers cannot call `SetConditionExplicitNull()` on a `WorkflowPreset`
- **AND** callers cannot call `ClearCondition()` on a `WorkflowPreset`

### Requirement: Workflow Step Compatibility
The system SHALL model workflow presets using the CMake workflow-step constraints.

The first workflow step SHALL reference a Configure preset.

Each subsequent workflow step SHALL reference a Build, Test, or Package preset whose `configurePreset` resolves to the same configure preset selected by the first step.

Workflow-step violations SHALL be representable as diagnostics associated with the workflow preset without requiring the workflow preset to participate in inheritance resolution.

#### Scenario: Validating workflow steps against the initial configure preset
- **GIVEN** a WorkflowPreset whose first step references Configure preset "cfg"
- **AND** its second step references Build preset "bld"
- **AND** Build preset "bld" resolves `configurePreset` to "cfg"
- **WHEN** the workflow preset is validated
- **THEN** the workflow preset is considered structurally valid

### Requirement: Condition Field Representation
For preset types that support `condition`, the system SHALL represent the field so it can distinguish:
- No local `condition` field was provided.
- The preset explicitly provided `condition: null`.
- The preset provided a parsed evaluable condition.

An explicit `condition: null` SHALL clear any inherited condition for the current preset and SHALL NOT be inherited by descendant presets.

#### Scenario: Explicit null clears the inherited condition chain
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and has `condition: null`
- **AND** preset `G` inherits from [`C`] and does not define `condition`
- **WHEN** the preset model resolves conditions
- **THEN** preset `C` has no effective evaluable condition
- **AND** preset `G` does not inherit an explicit null condition from `C`

### Requirement: Raw JSON And In-Preset Resolved State
For each preset, the system SHALL retain:
- The raw/original JSON object as read from disk.
- A current resolved-state object stored on the preset itself.

The resolved-state object SHALL:
- Use CMake preset field names as keys.
- Store the current resolved value for each tracked field after inheritance merge and macro expansion.
- Record whether each tracked field is unresolved, partially resolved, or fully resolved.
- Preserve structured fields using JSON when the field is not naturally a scalar string.
- Represent effective environment entries in preset-owned resolved state after environment merge and expansion, with one tracked resolved entry per environment key, stored under a single `environment` key in the resolved-state object using a nested object keyed by environment variable name.
- Track library-relevant macro-expandable scalar preset fields beyond `generator`, `installDir`, `binaryDir`, and `toolchainFile` as coverage is added, using the scalar allowlist inclusion criteria defined in the design.

The system SHALL NOT require a separate model-managed `ResolvedPreset` or `RawResolvedPreset` object to represent resolved state, and preset-owned resolved fields SHALL be the authoritative resolved-state interface for callers.

#### Scenario: Preserving a partially resolved field on the preset
- **GIVEN** a ConfigurePreset whose raw JSON `binaryDir` is `"${sourceDir}/build/${unknown}"`
- **WHEN** the preset refreshes its resolved state with a Macro Context that does not define `unknown`
- **THEN** the preset retains the original raw JSON value for `binaryDir`
- **AND** the preset's resolved state stores the current partially expanded `binaryDir` value
- **AND** the `binaryDir` entry is marked partially resolved

#### Scenario: Preserving a structured field on the preset
- **GIVEN** a ConfigurePreset whose raw JSON contains object-valued `cacheVariables`
- **WHEN** the preset refreshes its resolved state
- **THEN** the preset retains the original raw JSON `cacheVariables`
- **AND** the preset's resolved state can retain `cacheVariables` as structured JSON

#### Scenario: Tracking expanded environment entries on the preset
- **GIVEN** a ConfigurePreset whose effective environment contains `A="$env{HOME}"` and `B="$env{MISSING}"`
- **WHEN** the preset refreshes its resolved state with `HOME` defined and `MISSING` undefined
- **THEN** the preset's resolved state stores one tracked entry for `A` with the expanded value
- **AND** the entry for `A` is marked fully resolved
- **AND** the preset's resolved state stores one tracked entry for `B` with the unresolved macro preserved
- **AND** the entry for `B` is marked partially resolved

#### Scenario: Tracking additional scalar preset fields in resolved state
- **GIVEN** a ConfigurePreset whose raw JSON `cmakeExecutable` is `"${sourceDir}/tools/cmake"`
- **WHEN** the preset refreshes its resolved state with `sourceDir` defined
- **THEN** the preset's resolved state stores the expanded `cmakeExecutable` value
- **AND** the `cmakeExecutable` entry is marked fully resolved

### Requirement: Diagnostic-Friendly Resolved State
The preset resolved-state model SHALL preserve unresolved macro references exactly as produced by the library's macro-expansion rules.

The system SHALL NOT coerce missing `$env{}` / `$penv{}` references to empty strings for the sake of strict CMake emulation.

This requirement applies equally to top-level scalar fields and to resolved environment entries stored on the preset.

#### Scenario: Preserving a missing environment reference in resolved state
- **GIVEN** a ConfigurePreset whose raw JSON `binaryDir` is `"$env{MISSING}/build"`
- **WHEN** the preset refreshes its resolved state with no `MISSING` value in preset or parent environment
- **THEN** the preset's resolved state retains `"$env{MISSING}/build"` as the current value
- **AND** the `binaryDir` entry is not marked fully resolved

#### Scenario: Preserving a missing environment reference in a resolved environment entry
- **GIVEN** a ConfigurePreset whose effective environment contains `A="$env{MISSING}/bin"`
- **WHEN** the preset refreshes its resolved state with no `MISSING` value in preset or parent environment
- **THEN** the preset's resolved state retains `"$env{MISSING}/bin"` as the current value for `A`
- **AND** the resolved entry for `A` is not marked fully resolved

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
- **AND** the expansion result Status is `PartiallyExpanded`

### Requirement: Library-Relevant Expanded Fields
The system SHALL provide a minimal typed view of each preset sufficient to build and evaluate the graphs.

The base `Preset` class SHALL expose:
- `name` (string, required)
- `hidden` (bool, optional, defaults to false)
- `inherits` (list of strings, optional)
- `condition` (parsed condition declaration; may represent no local field, explicit `null`, or a Condition AST)
- `environment` (map of string to optional string, optional)

The `ConfigurePreset` class SHALL additionally expose:
- `generator` (string, optional)
- `installDir` (string, optional)

The `BuildPreset`, `TestPreset`, and `PackagePreset` classes SHALL additionally expose:
- `configurePreset` (string, optional)
- `inheritConfigureEnvironment` (bool, defaults to true for Build/Test/Package)

The `WorkflowPreset` class SHALL expose:
- `name` (string, required)
- `steps` (list of WorkflowStep, required)

Where `WorkflowStep` contains:
- `type` (enum: Configure, Build, Test, Package)
- `name` (string, the preset name to execute)

All other fields MAY be retained in the raw/original JSON and, when the library evaluates them, in the preset's current resolved state for this change.

#### Scenario: Minimal typed access for graph resolution
- **WHEN** a preset is added to the system
- **THEN** its name, inherits list, condition, and environment can be queried without requiring consumers to inspect raw JSON

#### Scenario: Type-safe preset retrieval
- **GIVEN** a ConfigurePreset named "cfg" added to the model
- **WHEN** a caller retrieves the preset
- **THEN** the caller can access it as a `ConfigurePreset` to read `generator`
- **AND** the caller can access it as a base `Preset` to read common fields

### Requirement: Cycle-Safe Effective Condition Resolution
The preset model SHALL resolve a preset's effective condition using standard `inherits` precedence without recursing indefinitely through cyclic inherits chains.

Effective-condition lookup SHALL:
- return the preset's local evaluable condition when one is present;
- treat local `condition: null` as clearing the current preset's inherited condition;
- skip inheriting an explicit `null` condition from an ancestor; and
- stop traversing any inherits path that re-enters a preset already being visited.

If every reachable inherits path is terminated by explicit `null`, missing parents, or a detected cycle before producing an evaluable condition, the effective condition SHALL be absent.

#### Scenario: ResolveCondition returns an inherited evaluable condition
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and does not define a local condition
- **WHEN** the preset model resolves the effective condition for `C`
- **THEN** the result is the evaluable condition inherited from `P0`

#### Scenario: ResolveCondition skips explicit null when traversing descendants
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and defines `condition: null`
- **AND** preset `G` inherits from [`C`] and does not define `condition`
- **WHEN** the preset model resolves the effective condition for `G`
- **THEN** the result is absent

#### Scenario: ResolveCondition terminates safely on an inherits cycle
- **GIVEN** preset `A` inherits from [`B`]
- **AND** preset `B` inherits from [`A`]
- **AND** neither preset defines a local evaluable condition
- **WHEN** the preset model resolves the effective condition for `A`
- **THEN** the lookup completes without unbounded recursion
- **AND** the result is absent

