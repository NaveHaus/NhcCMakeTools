## ADDED Requirements

### Requirement: Macro Context Storage
The system SHALL provide a Macro Context containing:
- A dictionary-like structure mapping macro names (strings) to their string values.
- A dictionary-like structure representing the preset environment (string -> string).
- A dictionary-like structure representing the parent environment (string -> string).

The system SHALL NOT read from the actual process environment. The "parent environment" map represents the process scope for `$env{}` and `$penv{}` expansion and SHALL be provided by the caller.

#### Scenario: Storing and retrieving a macro
- **WHEN** a macro "presetName" is set in the context
- **THEN** querying the context for "presetName" returns the stored value

### Requirement: Supported Macro Syntaxes
The system SHALL support expanding the following macro syntaxes in strings:
- `${macroName}`
- `$env{variableName}`
- `$penv{variableName}`

#### Scenario: Expanding strings with supported macro syntaxes
- **WHEN** a string "a-${presetName}-b-$env{X}-c-$penv{Y}" is expanded with context {"presetName": "p"}, preset environment {"X": "x"}, and parent environment {"Y": "y"}
- **THEN** the result is "a-p-b-x-c-y"

The system SHALL NOT implicitly populate system-provided CMake macros (e.g., `${sourceDir}` and `${hostSystemName}`). If such macros are to be expanded, they SHALL be provided explicitly by the caller via the macro map.

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
- **THEN** the result is "./vcpkg-root/scripts"

#### Scenario: $penv ignores preset environment
- **WHEN** a string "$penv{VCPKG_ROOT}/scripts" is expanded with preset environment {"VCPKG_ROOT": "./vcpkg-root"} and parent environment {"VCPKG_ROOT": "C:/vcpkg"}
- **THEN** the result is "C:/vcpkg/scripts"

#### Scenario: Missing environment variable remains unresolved
- **WHEN** a string "prefix-$env{DOES_NOT_EXIST}-suffix" is expanded and `DOES_NOT_EXIST` is not present in preset or parent environment
- **THEN** the result is "prefix-$env{DOES_NOT_EXIST}-suffix"
- **AND** the system can optionally indicate that the expansion was only partial

### Requirement: String Expansion
The system SHALL expand strings containing valid CMake macros using the provided Macro Context.

#### Scenario: Expanding a string with known macros
- **WHEN** a string "build-${presetName}-${generator}" is expanded with context {"presetName": "default", "generator": "Ninja"}
- **THEN** the result is "build-default-Ninja"

### Requirement: Graceful Partial Expansion
The system SHALL expand known macros and leave unknown macros unexpanded, rather than treating partial expansion as a hard error.

#### Scenario: Expanding a string with missing macros
- **WHEN** a string "build-${presetName}-${unknownMacro}" is expanded with context {"presetName": "default"}
- **THEN** the result is "build-default-${unknownMacro}"
- **AND** the system can optionally indicate that the expansion was only partial
