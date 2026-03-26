## ADDED Requirements

### Requirement: Condition Evaluation Interface
The system SHALL define a base Condition type capable of evaluating itself against a given Macro Context, returning true, false, or an indeterminate state if macros are missing.

#### Scenario: Evaluating a condition missing a macro
- **WHEN** a Condition depends on the macro `${compiler}` and the context does not contain it
- **THEN** the evaluation result is indeterminate (unknown)

### Requirement: Equality Condition
The system SHALL support an "equals" and "notEquals" condition comparing a left-hand string and a right-hand string, both of which may contain macros.

#### Scenario: Evaluating an equals condition
- **WHEN** an "equals" condition compares "${os}" and "windows" using a context where "os" is "windows"
- **THEN** the condition evaluates to true

### Requirement: Boolean Logic Conditions
The system SHALL support logical grouping conditions: "anyOf", "allOf", and "not", which contain other Condition objects.

#### Scenario: Evaluating an allOf condition
- **WHEN** an "allOf" condition contains two sub-conditions, and one evaluates to false
- **THEN** the "allOf" condition short-circuits and evaluates to false
