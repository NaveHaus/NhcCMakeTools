## MODIFIED Requirements

### Requirement: Raw JSON And In-Preset Resolved State
For each preset, the system SHALL retain:
- The raw/original JSON object as read from disk.
- A current resolved-state object stored on the preset itself.

The resolved-state object SHALL:
- Use CMake preset field names as keys.
- Store the current resolved value for each tracked field after inheritance merge and macro expansion.
- Record whether each tracked field is unresolved, partially resolved, or fully resolved.
- Preserve structured fields using JSON when the field is not naturally a scalar string.
- Represent effective environment entries in preset-owned resolved state after environment merge and expansion, with one tracked resolved entry per environment key, stored under a single `environment` key in the resolved-state object using a nested object keyed by environment variable name.
- Track library-relevant macro-expandable scalar preset fields beyond `generator`, `installDir`, `binaryDir`, and `toolchainFile` as coverage is added, using the scalar allowlist inclusion criteria defined in the design.

The system SHALL NOT require a separate model-managed `ResolvedPreset` or `RawResolvedPreset` object to represent resolved state, and preset-owned resolved fields SHALL be the authoritative resolved-state interface for callers.

#### Scenario: Preserving a partially resolved field on the preset
- **GIVEN** a ConfigurePreset whose raw JSON `binaryDir` is `"${sourceDir}/build/${unknown}"`
- **WHEN** the preset refreshes its resolved state with a Macro Context that does not define `unknown`
- **THEN** the preset retains the original raw JSON value for `binaryDir`
- **AND** the preset's resolved state stores the current partially expanded `binaryDir` value
- **AND** the `binaryDir` entry is marked partially resolved

#### Scenario: Preserving a structured field on the preset
- **GIVEN** a ConfigurePreset whose raw JSON contains object-valued `cacheVariables`
- **WHEN** the preset refreshes its resolved state
- **THEN** the preset retains the original raw JSON `cacheVariables`
- **AND** the preset's resolved state can retain `cacheVariables` as structured JSON

#### Scenario: Tracking expanded environment entries on the preset
- **GIVEN** a ConfigurePreset whose effective environment contains `A="$env{HOME}"` and `B="$env{MISSING}"`
- **WHEN** the preset refreshes its resolved state with `HOME` defined and `MISSING` undefined
- **THEN** the preset's resolved state stores one tracked entry for `A` with the expanded value
- **AND** the entry for `A` is marked fully resolved
- **AND** the preset's resolved state stores one tracked entry for `B` with the unresolved macro preserved
- **AND** the entry for `B` is marked partially resolved

#### Scenario: Tracking additional scalar preset fields in resolved state
- **GIVEN** a ConfigurePreset whose raw JSON `cmakeExecutable` is `"${sourceDir}/tools/cmake"`
- **WHEN** the preset refreshes its resolved state with `sourceDir` defined
- **THEN** the preset's resolved state stores the expanded `cmakeExecutable` value
- **AND** the `cmakeExecutable` entry is marked fully resolved

### Requirement: Diagnostic-Friendly Resolved State
The preset resolved-state model SHALL preserve unresolved macro references exactly as produced by the library's macro-expansion rules.

The system SHALL NOT coerce missing `$env{}` / `$penv{}` references to empty strings for the sake of strict CMake emulation.

This requirement applies equally to top-level scalar fields and to resolved environment entries stored on the preset.

#### Scenario: Preserving a missing environment reference in resolved state
- **GIVEN** a ConfigurePreset whose raw JSON `binaryDir` is `"$env{MISSING}/build"`
- **WHEN** the preset refreshes its resolved state with no `MISSING` value in preset or parent environment
- **THEN** the preset's resolved state retains `"$env{MISSING}/build"` as the current value
- **AND** the `binaryDir` entry is not marked fully resolved

#### Scenario: Preserving a missing environment reference in a resolved environment entry
- **GIVEN** a ConfigurePreset whose effective environment contains `A="$env{MISSING}/bin"`
- **WHEN** the preset refreshes its resolved state with no `MISSING` value in preset or parent environment
- **THEN** the preset's resolved state retains `"$env{MISSING}/bin"` as the current value for `A`
- **AND** the resolved entry for `A` is not marked fully resolved
