## ADDED Requirements

### Requirement: Condition Evaluation Interface
The system SHALL define a base Condition type capable of evaluating itself against a given Macro Context, returning true, false, or an indeterminate state if macros are missing.

If any macro expansion required to evaluate a Condition is partial (e.g., due to missing macro or environment values), the evaluation result SHALL be indeterminate (unknown).

#### Scenario: Evaluating a condition missing a macro
- **WHEN** a Condition depends on the macro `${unknownMacro}` and the context does not contain it
- **THEN** the evaluation result is indeterminate (unknown)

#### Scenario: Evaluating a condition missing an environment value
- **WHEN** a Condition depends on `$env{DOES_NOT_EXIST}` and neither preset nor parent environment contains it
- **THEN** the evaluation result is indeterminate (unknown)

### Requirement: Constant Condition
The system SHALL support a constant condition that always evaluates to a given boolean value.

#### Scenario: Evaluating a const condition
- **WHEN** a const condition has value true
- **THEN** the condition evaluates to true

### Requirement: Equality Condition
The system SHALL support an "equals" and "notEquals" condition comparing a left-hand string and a right-hand string, both of which may contain macros.

#### Scenario: Evaluating an equals condition
- **WHEN** an "equals" condition compares "${presetName}" and "default" using a context where "presetName" is "default"
- **THEN** the condition evaluates to true

### Requirement: List Membership Conditions
The system SHALL support an "inList" and "notInList" condition comparing a string against a list of strings, where all strings may contain macros.

#### Scenario: Evaluating an inList condition
- **WHEN** an "inList" condition compares "${presetName}" against ["default", "other"] using a context where "presetName" is "default"
- **THEN** the condition evaluates to true

#### Scenario: Evaluating a notInList condition
- **WHEN** a "notInList" condition compares "${presetName}" against ["other"] using a context where "presetName" is "default"
- **THEN** the condition evaluates to true

### Requirement: Regex Conditions
The system SHALL support a "matches" and "notMatches" condition comparing a string against a regex pattern, where both may contain macros.

#### Scenario: Evaluating a matches condition
- **WHEN** a "matches" condition compares "${presetName}" against "def.*" using a context where "presetName" is "default"
- **THEN** the condition evaluates to true

### Requirement: Boolean Logic Conditions
The system SHALL support logical grouping conditions: "anyOf", "allOf", and "not", which contain other Condition objects.

#### Scenario: Evaluating an allOf condition
- **WHEN** an "allOf" condition contains two sub-conditions, and one evaluates to false
- **THEN** the "allOf" condition short-circuits and evaluates to false
