## MODIFIED Requirements

### Requirement: Workflow Step Compatibility
The system SHALL model workflow presets using the CMake workflow-step constraints.

The first workflow step SHALL reference a Configure preset.

Each subsequent workflow step SHALL reference a Build, Test, or Package preset whose `configurePreset` resolves to the same configure preset selected by the first step.

Workflow-step violations SHALL be representable as diagnostics associated with the workflow preset.

When workflow-step validation fails, the workflow preset itself SHALL be marked Unresolved with reason `InvalidWorkflowSteps`.

Representing workflow-step violations on the workflow preset SHALL NOT require the workflow preset to participate in inheritance resolution.

#### Scenario: Validating workflow steps against the initial configure preset
- **GIVEN** a WorkflowPreset whose first step references Configure preset "cfg"
- **AND** its second step references Build preset "bld"
- **AND** Build preset "bld" resolves `configurePreset` to "cfg"
- **WHEN** the workflow preset is validated
- **THEN** the workflow preset is considered structurally valid

#### Scenario: Reporting invalid workflow structure on the workflow preset
- **GIVEN** a WorkflowPreset whose first step references Configure preset "cfg"
- **AND** a later workflow step references Build preset "bld-other"
- **AND** Build preset "bld-other" resolves `configurePreset` to "cfg-other"
- **WHEN** the workflow preset is validated
- **THEN** the workflow preset is marked Unresolved with reason `InvalidWorkflowSteps`
- **AND** the workflow preset retains diagnostics describing the mismatched step
