## ADDED Requirements

### Requirement: Macro Context Storage
The system SHALL provide a dictionary-like structure mapping macro names (strings) to their string values.

#### Scenario: Storing and retrieving a macro
- **WHEN** a macro "os" is set to "windows" in the context
- **THEN** querying the context for "os" returns "windows"

### Requirement: String Expansion
The system SHALL expand strings containing valid CMake macros (e.g., `${macroName}`) using the provided Macro Context.

#### Scenario: Expanding a string with known macros
- **WHEN** a string "build-${os}-${arch}" is expanded with context {"os": "linux", "arch": "x64"}
- **THEN** the result is "build-linux-x64"

### Requirement: Graceful Partial Expansion
The system SHALL expand known macros and leave unknown macros unexpanded, rather than treating partial expansion as a hard error.

#### Scenario: Expanding a string with missing macros
- **WHEN** a string "build-${os}-${compiler}" is expanded with context {"os": "linux"}
- **THEN** the result is "build-linux-${compiler}"
- **AND** the system can optionally indicate that the expansion was only partial
