## ADDED Requirements

### Requirement: NhcTestLib target is available only for test builds
The build system SHALL define a target named `NhcTestLib` only when `BUILD_TESTING=ON`.
The `NhcTestLib` target SHALL be a static library.

#### Scenario: BUILD_TESTING is enabled
- **WHEN** the project is configured with `BUILD_TESTING=ON`
- **THEN** the target `NhcTestLib` exists and is a static library

#### Scenario: BUILD_TESTING is disabled
- **WHEN** the project is configured with `BUILD_TESTING=OFF`
- **THEN** the target `NhcTestLib` does not exist

### Requirement: NhcTestLib provides the test executable entry point
`NhcTestLib` SHALL provide a single `main(int, char**)` entry point that is used by all test executables in this repository.

#### Scenario: A test executable links NhcTestLib
- **WHEN** a test executable is linked against `NhcTestLib`
- **THEN** the test executable links successfully without defining its own `main()`

### Requirement: NhcTestLib centralizes Catch2 integration for tests
`NhcTestLib` SHALL provide Catch2 to all test executables that link it.
Tests in this repository SHALL NOT be required to directly link Catch2 targets to compile and link.

#### Scenario: A test source includes Catch2 headers
- **WHEN** a test executable links `NhcTestLib`
- **THEN** the test sources can include Catch2 headers and link successfully

### Requirement: Catch2 is not required for non-test builds
The project SHALL NOT require locating Catch2 when `BUILD_TESTING=OFF`.

#### Scenario: Non-test configure does not need Catch2
- **WHEN** the project is configured with `BUILD_TESTING=OFF`
- **THEN** configuring does not require Catch2 to be installed or discoverable
