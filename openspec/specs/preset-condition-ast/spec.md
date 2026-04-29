## Purpose

Define condition parsing and evaluation behavior for CMake preset condition declarations.

## Requirements

### Requirement: Condition Evaluation Interface
The system SHALL define a base Condition type capable of evaluating itself against a given Macro Context, returning true, false, or an indeterminate state if macros are missing.

If any macro expansion required to evaluate a Condition yields an ExpansionResult with Status `PartiallyExpanded` (e.g., due to missing macro or environment values), the evaluation result SHALL be indeterminate (unknown).

#### Scenario: Evaluating a condition missing a macro
- **WHEN** a Condition depends on the macro `${unknownMacro}` and the context does not contain it
- **THEN** the evaluation result is indeterminate (unknown)

#### Scenario: Evaluating a condition missing an environment value
- **WHEN** a Condition depends on `$env{DOES_NOT_EXIST}` and neither preset nor parent environment contains it
- **THEN** the evaluation result is indeterminate (unknown)

### Requirement: Condition JSON Wire Forms
The system SHALL parse a preset `condition` JSON value according to the CMake wire format.

- A boolean value SHALL parse as a constant condition with the same boolean value.
- A top-level `null` value SHALL parse as an explicit enabled, non-inheritable condition marker.
- A JSON object SHALL parse as a typed condition object.
- Nested conditions inside `condition` / `conditions` MAY be booleans or objects, but SHALL NOT be `null`.

#### Scenario: Parsing a boolean condition value
- **WHEN** the parser receives the JSON value `true` as a top-level preset `condition`
- **THEN** it produces a constant condition that evaluates to true

#### Scenario: Parsing a null condition value
- **WHEN** the parser receives the JSON value `null` as a top-level preset `condition`
- **THEN** it produces an explicit enabled, non-inheritable condition marker

### Requirement: Condition Object Parsing
The system SHALL parse condition objects using the CMake-defined `type` field and the type-specific member names associated with that type.

- `const` SHALL use `value`
- `equals` and `notEquals` SHALL use `lhs` and `rhs`
- `inList` and `notInList` SHALL use `string` and `list`
- `matches` and `notMatches` SHALL use `string` and `regex`
- `anyOf` and `allOf` SHALL use `conditions`
- `not` SHALL use `condition`

#### Scenario: Parsing an equals condition object
- **WHEN** the parser receives `{"type":"equals","lhs":"${presetName}","rhs":"default"}`
- **THEN** it produces an equals condition with `lhs` and `rhs` taken from those fields

#### Scenario: Parsing an anyOf condition object
- **WHEN** the parser receives `{"type":"anyOf","conditions":[false,{"type":"const","value":true}]}`
- **THEN** it produces an anyOf condition containing those two parsed child conditions

### Requirement: Condition Parse Failure Reporting
If a `condition` value does not conform to the supported CMake wire format, the system SHALL report condition-parse failure instead of synthesizing a placeholder AST.

#### Scenario: Rejecting a nested null condition
- **WHEN** the parser receives `{"type":"not","condition":null}`
- **THEN** it reports condition-parse failure

#### Scenario: Rejecting an unknown condition type
- **WHEN** the parser receives `{"type":"platformEquals","lhs":"x","rhs":"y"}`
- **THEN** it reports condition-parse failure

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

#### Scenario: Evaluating a notEquals condition
- **WHEN** a "notEquals" condition compares "${presetName}" and "other" using a context where "presetName" is "default"
- **THEN** the condition evaluates to true

#### Scenario: Evaluating a notEquals condition that is false
- **WHEN** a "notEquals" condition compares "${presetName}" and "default" using a context where "presetName" is "default"
- **THEN** the condition evaluates to false

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
The system SHALL support logical grouping conditions: "anyOf", "allOf", and "not", which contain other parsed Condition values.

#### Scenario: Evaluating an allOf condition
- **WHEN** an "allOf" condition contains two sub-conditions, and one evaluates to false
- **THEN** the "allOf" condition short-circuits and evaluates to false

#### Scenario: Evaluating an anyOf condition that is true
- **WHEN** an "anyOf" condition contains two sub-conditions, and one evaluates to true
- **THEN** the "anyOf" condition short-circuits and evaluates to true

#### Scenario: Evaluating an anyOf condition that is false
- **WHEN** an "anyOf" condition contains two sub-conditions, and both evaluate to false
- **THEN** the "anyOf" condition evaluates to false

#### Scenario: Evaluating an anyOf condition with unknown
- **WHEN** an "anyOf" condition contains a false condition and an unknown condition
- **THEN** the "anyOf" condition evaluates to unknown

#### Scenario: Evaluating a not condition
- **WHEN** a "not" condition wraps a condition that evaluates to true
- **THEN** the "not" condition evaluates to false

#### Scenario: Evaluating a not condition with false
- **WHEN** a "not" condition wraps a condition that evaluates to false
- **THEN** the "not" condition evaluates to true

#### Scenario: Evaluating a not condition with unknown
- **WHEN** a "not" condition wraps a condition that evaluates to unknown
- **THEN** the "not" condition evaluates to unknown
