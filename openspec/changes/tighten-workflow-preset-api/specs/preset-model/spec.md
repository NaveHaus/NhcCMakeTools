## MODIFIED Requirements

### Requirement: Preset Type Hierarchy
The system SHALL implement a class hierarchy for preset types:
- A base `Preset` class SHALL define fields common to Configure, Build, Test, and Package presets: `name`, `hidden`, `inherits`, parsed `condition` declaration, `environment`.
- `ConfigurePreset` SHALL derive from `Preset` and add: `generator`, `installDir`.
- `BuildPreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `TestPreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `PackagePreset` SHALL derive from `Preset` and add: `configurePreset`, `inheritConfigureEnvironment`.
- `WorkflowPreset` MAY derive from `Preset` for implementation convenience, but the typed API SHALL expose only workflow-supported accessors. Per the CMake specification, workflow presets support `name`, `steps`, `displayName`, `description`, and `vendor`. Callers SHALL NOT be given typed accessors for inherited `hidden`, `inherits`, `condition`, `environment`, or helpers that expose workflow condition state.

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
- **AND** it exposes `name`, `steps`, `displayName`, `description`, and `vendor`

#### Scenario: WorkflowPreset omits inherited condition-state helpers
- **GIVEN** a WorkflowPreset named "wf"
- **WHEN** the preset is queried through the typed workflow API
- **THEN** callers cannot inspect workflow condition state through inherited helper APIs
- **AND** callers cannot distinguish missing, explicit-null, or parsed condition values on a `WorkflowPreset`
