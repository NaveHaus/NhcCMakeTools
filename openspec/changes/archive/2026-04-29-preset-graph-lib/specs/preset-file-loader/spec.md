## ADDED Requirements

### Requirement: Load File Content
The system SHALL define a file loader abstraction that loads a text file given a file path and returns its complete contents as a string.

#### Scenario: Loading a file by absolute path
- **WHEN** the file loader is asked to load an absolute path that exists
- **THEN** it returns the file contents as a string

### Requirement: Failure Reporting
The file loader SHALL report a failure when the requested file cannot be read, including whether the failure was due to the file not existing.

#### Scenario: Loading a missing file
- **WHEN** the file loader is asked to load a path that does not exist
- **THEN** it reports a failure indicating `FileDoesNotExist`
