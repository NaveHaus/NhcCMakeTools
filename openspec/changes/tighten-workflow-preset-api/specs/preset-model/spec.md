## MODIFIED Requirements

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
