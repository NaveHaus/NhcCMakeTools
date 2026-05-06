## ADDED Requirements

### Requirement: Reload Attempts Recompute File Unresolved State
The Presets Graph Manager SHALL clear a file node's unresolved load state before retrying that file during a later `ApplyContext()` call.

If the reload attempt succeeds, the file node SHALL remain cleared of any prior unresolved load reason from an earlier failed attempt.

If the reload attempt fails, the file node SHALL be left marked Unresolved with the reason assigned by the current reload attempt, not any reason carried over from a prior attempt.

#### Scenario: Successful reload clears stale unresolved file state
- **GIVEN** a previous `ApplyContext()` call marked a file node Unresolved with reason `FileDoesNotExist`
- **AND** the file is made available before the next `ApplyContext()` call
- **WHEN** the Manager retries loading that file on the later `ApplyContext()` call
- **THEN** the Manager clears the prior unresolved load state before the retry
- **AND** the file node is not left marked Unresolved with reason `FileDoesNotExist` after the successful reload

#### Scenario: Failed reload after reset records current-attempt reason
- **GIVEN** a previous `ApplyContext()` call marked a file node Unresolved with reason `ParseError`
- **AND** the file is removed before the next `ApplyContext()` call
- **WHEN** the Manager retries loading that file on the later `ApplyContext()` call
- **THEN** the Manager clears the prior unresolved load state before the retry
- **AND** the file node is left marked Unresolved with reason `FileDoesNotExist`
- **AND** the file node is not left marked Unresolved with the stale reason `ParseError` from the prior attempt

## MODIFIED Requirements

### Requirement: Supported Preset File Version
The Presets Graph Manager SHALL determine the maximum supported preset file `version` based on the configured simulated CMake version.

At minimum, the following mappings SHALL be supported:
- CMake 3.19 supports preset file version 1
- CMake 3.20 supports preset file version 2
- CMake 3.21 supports preset file version 3
- CMake 3.23 supports preset file version 4
- CMake 3.24 supports preset file version 5
- CMake 3.25 supports preset file version 6
- CMake 3.27 supports preset file version 7
- CMake 3.28 supports preset file version 8
- CMake 3.30 supports preset file version 9
- CMake 3.31 supports preset file version 10
- CMake 4.0 supports preset file version 10
- CMake 4.1 supports preset file version 10
- CMake 4.2 supports preset file version 10
- CMake 4.3 supports preset file version 11

#### Scenario: Deriving supported preset file version for CMake 3.30
- **GIVEN** simulated CMake version 3.30.0
- **WHEN** the Manager computes the supported preset file version
- **THEN** it reports supported preset file version 9

#### Scenario: Deriving supported preset file version for CMake 4.2
- **GIVEN** simulated CMake version 4.2.0
- **WHEN** the Manager computes the supported preset file version
- **THEN** it reports supported preset file version 10

#### Scenario: Deriving supported preset file version for CMake 4.3
- **GIVEN** simulated CMake version 4.3.0
- **WHEN** the Manager computes the supported preset file version
- **THEN** it reports supported preset file version 11
