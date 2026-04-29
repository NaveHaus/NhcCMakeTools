## ADDED Requirements

### Requirement: Macro Context Storage
The system SHALL provide a Macro Context containing:
- A dictionary-like structure mapping macro names (strings) to their string values.
- A dictionary-like structure representing the preset environment (string -> string).
- A dictionary-like structure representing the parent environment (string -> string).

The system SHALL NOT read from the actual process environment. The "parent environment" map represents the process scope for `$env{}` and `$penv{}` expansion and SHALL be provided by the caller.

The macro map in the Macro Context SHALL be caller-controlled, and the graph manager MAY populate or update it with preset-associated values during graph resolution (e.g., `${presetName}` for the active preset).

#### Scenario: Storing and retrieving a macro
- **WHEN** a macro "presetName" is set in the context
- **THEN** querying the context for "presetName" returns the stored value

### Requirement: Supported Macro Syntaxes
The string expansion operation SHALL support expanding the following macro syntaxes in strings:
- `${macroName}`
- `$env{variableName}`
- `$penv{variableName}`

#### Scenario: Expanding strings with supported macro syntaxes
- **WHEN** a string "a-${presetName}-b-$env{X}-c-$penv{Y}" is expanded with context {"presetName": "p"}, preset environment {"X": "x"}, and parent environment {"Y": "y"}
- **THEN** the ExpansionResult ExpandedString is "a-p-b-x-c-y"
- **AND** the ExpansionResult Status is `FullyExpanded`
- **AND** the ExpansionResult UnresolvedTokens is empty

Macro values in the Macro Context SHALL be populated explicitly either by the caller or by the graph manager (when values can be derived from known preset and file state).

The system SHALL NOT read from the host system to implicitly populate macros like `${sourceDir}` and `${hostSystemName}`. If such macros are to be expanded, they SHALL be provided explicitly by the caller via the macro map.

#### Scenario: Graph manager injects derived macros
- **GIVEN** a file node "./a/b/c/CMakePresets.json"
- **AND** a preset "p" belonging to that file
- **WHEN** the graph manager resolves fields within that file and preset
- **THEN** it can inject `${fileDir}` as "./a/b/c" and `${presetName}` as "p" into the Macro Context

### Requirement: Environment Macro Expansion
When expanding `$env{variableName}`, the system SHALL resolve the value in this order:
1. If the preset environment map contains `variableName`, use that value.
2. Else, if the parent environment map contains `variableName`, use that value.
3. Else, leave the macro unexpanded.

When expanding `$penv{variableName}`, the system SHALL resolve the value in this order:
1. If the parent environment map contains `variableName`, use that value.
2. Else, leave the macro unexpanded.

#### Scenario: $env prefers preset environment
- **WHEN** a string "$env{VCPKG_ROOT}/scripts" is expanded with preset environment {"VCPKG_ROOT": "./vcpkg-root"} and parent environment {"VCPKG_ROOT": "C:/vcpkg"}
- **THEN** the ExpansionResult ExpandedString is "./vcpkg-root/scripts"
- **AND** the ExpansionResult Status is `FullyExpanded`
- **AND** the ExpansionResult UnresolvedTokens is empty

#### Scenario: $penv ignores preset environment
- **WHEN** a string "$penv{VCPKG_ROOT}/scripts" is expanded with preset environment {"VCPKG_ROOT": "./vcpkg-root"} and parent environment {"VCPKG_ROOT": "C:/vcpkg"}
- **THEN** the ExpansionResult ExpandedString is "C:/vcpkg/scripts"
- **AND** the ExpansionResult Status is `FullyExpanded`
- **AND** the ExpansionResult UnresolvedTokens is empty

#### Scenario: Missing environment variable remains unresolved
- **WHEN** a string "prefix-$env{DOES_NOT_EXIST}-suffix" is expanded and `DOES_NOT_EXIST` is not present in preset or parent environment
- **THEN** the ExpansionResult ExpandedString is "prefix-$env{DOES_NOT_EXIST}-suffix"
- **AND** the ExpansionResult Status is `PartiallyExpanded`
- **AND** the ExpansionResult UnresolvedTokens contains "$env{DOES_NOT_EXIST}"

### Requirement: String Expansion
The system SHALL provide a string expansion operation that expands strings containing supported CMake preset macros using the provided Macro Context.

The string expansion operation SHALL return an ExpansionResult containing:
- `ExpandedString`: the expanded string
- `Status`: `FullyExpanded` or `PartiallyExpanded`
- `UnresolvedTokens`: a list of unexpanded macro tokens still present in `ExpandedString`

`Status` SHALL be `FullyExpanded` if and only if `UnresolvedTokens` is empty.

#### Scenario: Expanding a string with known macros
- **WHEN** a string "build-${presetName}-${generator}" is expanded with context {"presetName": "default", "generator": "Ninja"}
- **THEN** the ExpansionResult ExpandedString is "build-default-Ninja"
- **AND** the ExpansionResult Status is `FullyExpanded`
- **AND** the ExpansionResult UnresolvedTokens is empty

#### Scenario: Reporting unresolved tokens
- **WHEN** a string "x-${unknownMacro}-y" is expanded with context {"presetName": "default"}
- **THEN** the ExpansionResult ExpandedString is "x-${unknownMacro}-y"
- **AND** the ExpansionResult Status is `PartiallyExpanded`
- **AND** the ExpansionResult UnresolvedTokens contains "${unknownMacro}"

### Requirement: Graceful Partial Expansion
The system SHALL expand known macros and leave unknown macros unexpanded, rather than treating partial expansion as a hard error.

This behavior is the library contract for v1, even where CMake command-line evaluation would substitute an empty string for a missing environment reference.

#### Scenario: Expanding a string with missing macros
- **WHEN** a string "build-${presetName}-${unknownMacro}" is expanded with context {"presetName": "default"}
- **THEN** the ExpansionResult ExpandedString is "build-default-${unknownMacro}"
- **AND** the ExpansionResult Status is `PartiallyExpanded`
- **AND** the ExpansionResult UnresolvedTokens contains "${unknownMacro}"
