## MODIFIED Requirements

### Requirement: Workflow Preset Validation
When the Manager refreshes workflow presets in `PresetModel`, it SHALL validate each workflow preset against the typed presets already present in the model.

The first workflow step SHALL reference a Configure preset.

Each subsequent workflow step SHALL reference a Build, Test, or Package preset whose `configurePreset` resolves to the same configure preset selected by the first step.

If workflow validation fails, the Manager SHALL keep the workflow preset in `PresetModel`, SHALL mark that workflow preset Unresolved with reason `InvalidWorkflowSteps`, SHALL surface a non-fatal workflow validation diagnostic for that workflow preset, and SHALL continue refreshing other presets.

Workflow validation SHALL NOT add workflow presets to the Inheritance Graph or otherwise require workflow presets to participate in inheritance resolution.

#### Scenario: Reporting a workflow step with a mismatched configure preset
- **GIVEN** a workflow preset whose first step references Configure preset `cfg`
- **AND** a later workflow step references Build preset `bld-other`
- **AND** `bld-other.configurePreset` resolves to `cfg-other`
- **WHEN** the Manager validates the workflow preset
- **THEN** the workflow preset remains queryable in `PresetModel`
- **AND** the workflow preset is marked Unresolved with reason `InvalidWorkflowSteps`
- **AND** the Manager records a workflow validation diagnostic for the mismatched step

#### Scenario: Invalid workflow validation does not create inheritance participation
- **GIVEN** a workflow preset with invalid workflow steps
- **WHEN** the Manager completes workflow validation
- **THEN** the workflow preset is not added to the Inheritance Graph
- **AND** other preset ingestion and refresh work continues
