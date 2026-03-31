## 1. Setup

- [x] 1.1 Add CMake target for a static library `NhcPresetGraph`
- [x] 1.2 Create test directory `tests/PresetsGraph`
- [x] 1.3 Add `tests/PresetsGraph/CMakeLists.txt` with `nhc_add_test_executable(... USES NhcTestLib)` entries for each `*Tests.cpp`
- [x] 1.4 Wire `tests/PresetsGraph/CMakeLists.txt` into the parent tests CMake configuration
- [x] 1.5 Add `nlohmann/json` dependency for `NhcPresetGraph`
- [x] 1.6 Add a file loader abstraction for reading preset files as strings

Notes:
- The capabilities listed in `openspec/changes/preset-graph-lib/proposal.md` are implemented as modules within the `NhcPresetGraph` library.

## 2. preset-macro-context

- [x] 2.1 **RED**: In a new `tests/PresetsGraph/MacroContextTests.cpp`, write a failing test for storing and retrieving a macro value.
- [x] 2.2 **GREEN**: Implement the `MacroContext` class and minimal code to pass the test.
- [x] 2.3 **RED**: Add a failing test for expanding a string with a known macro returning `FullyExpanded` and no unresolved tokens.
- [x] 2.4 **GREEN**: Implement the expansion logic to pass the test.
- [x] 2.5 **RED**: Add a failing test for gracefully handling a string with an unknown macro by returning `PartiallyExpanded` and reporting the unresolved token.
- [x] 2.6 **GREEN**: Implement the partial expansion logic to pass the test.
- [x] 2.7 **RED**: Add a failing test verifying that `$env{NAME}` prefers the preset environment value over the parent environment value and returns `FullyExpanded`.
- [x] 2.8 **GREEN**: Implement `$env{}` expansion logic to pass the test.
- [x] 2.9 **RED**: Add a failing test verifying that `$penv{NAME}` uses only the parent environment value and returns `FullyExpanded`.
- [x] 2.10 **GREEN**: Implement `$penv{}` expansion logic to pass the test.
- [x] 2.10a **RED**: Add a failing test verifying that when the MacroContext parent environment map does not contain a variable, `$penv{VAR}` remains unresolved (not replaced with an empty string).
- [x] 2.10b **GREEN**: Implement the missing-parent-variable behavior to pass the test.
- [x] 2.10c **RED**: Add a failing test verifying that when neither preset nor parent environment defines a variable, `$env{VAR}` remains unresolved (not replaced with an empty string).
- [x] 2.10d **GREEN**: Implement the missing-variable behavior to pass the test.
- [x] 2.11 **REFACTOR**: Review `MacroContext` for clarity and correctness.

## 3. preset-graph-core

- [x] 3.1 **RED**: In a new `tests/PresetsGraph/CoreGraphTests.cpp`, write a failing test for adding a node payload and receiving a unique ID.
- [x] 3.2 **GREEN**: Implement the base `Graph` class with node addition logic to pass the test.
- [x] 3.3 **RED**: Add a failing test for creating a directed edge between two nodes.
- [x] 3.4 **GREEN**: Implement edge tracking logic to pass the test.
- [x] 3.5 **RED**: Add a failing test that asserts a cycle is detected when a cyclic edge is added.
- [x] 3.6 **GREEN**: Implement cycle detection logic to pass the test.
- [x] 3.7 **REFACTOR**: Review core graph structures.

## 4. preset-condition-ast

- [x] 4.1 **RED**: In a new `tests/PresetsGraph/ConditionTests.cpp`, write a failing test for a simple `EqualsCondition` that should evaluate to true.
- [x] 4.2 **GREEN**: Implement the `Condition` interface and `EqualsCondition` class to pass the test.
- [x] 4.3 **RED**: Add a failing test for a condition that cannot be evaluated due to a missing macro.
- [x] 4.4 **GREEN**: Implement the logic to return an `Unknown` or indeterminate state to pass the test.
- [x] 4.5 **RED**: Add a failing test for a logical `AllOfCondition`.
- [x] 4.6 **GREEN**: Implement the `AllOfCondition` class to pass the test.
- [x] 4.7 **RED**: Add a failing test for `ConstCondition`.
- [x] 4.8 **GREEN**: Implement the `ConstCondition` class to pass the test.
- [x] 4.9 **RED**: Add a failing test for `InListCondition`.
- [x] 4.10 **GREEN**: Implement the `InListCondition` class to pass the test.
- [x] 4.11 **RED**: Add a failing test for `NotInListCondition`.
- [x] 4.12 **GREEN**: Implement the `NotInListCondition` class to pass the test.
- [x] 4.13 **RED**: Add a failing test for `MatchesCondition`.
- [x] 4.14 **GREEN**: Implement the `MatchesCondition` class to pass the test.
- [x] 4.15 **RED**: Add a failing test for `NotMatchesCondition`.
- [x] 4.16 **GREEN**: Implement the `NotMatchesCondition` class to pass the test.
- [x] 4.17 **REFACTOR**: Review `Condition` AST classes.

## 5. preset-include-graph

- [x] 5.1 **RED**: In a new `tests/PresetsGraph/IncludeGraphTests.cpp`, write a failing test for adding a `FilePayload`.
- [x] 5.2 **GREEN**: Implement the `PresetIncludeGraph` class and node addition to pass the test.
- [x] 5.3 **RED**: Add a failing test for computing an `Unresolved` state when a node has a pending include.
- [x] 5.4 **GREEN**: Implement the state computation logic to pass the test.
- [x] 5.5 **RED**: Add a failing test for resolving a pending include into an edge when the macro context is sufficient.
- [x] 5.6 **GREEN**: Implement the include resolution logic to pass the test.
- [x] 5.7 **REFACTOR**: Review `PresetIncludeGraph`.
- [x] 5.8 **RED**: Add a failing test for marking a file node as Unresolved with reason `FileDoesNotExist`.
- [x] 5.9 **GREEN**: Implement the unresolved-reason tracking to pass the test.
- [x] 5.10 **RED**: Add a failing test verifying that an include string containing `$env{...}` marks the file node Unresolved with reason `UnsupportedMacro`.
- [x] 5.11 **GREEN**: Implement the `$env{}` rejection logic to pass the test.
- [x] 5.12 **RED**: Add a failing test verifying that an include string containing `${presetName}` marks the file node Unresolved with reason `UnsupportedMacro`.
- [x] 5.13 **GREEN**: Implement the preset-specific macro rejection logic to pass the test.
- [x] 5.14 **RED**: Add a failing test verifying that an include string containing `${unknown}` remains unexpanded and marks the file node Unresolved with reason `MissingMacro`.
- [x] 5.15 **GREEN**: Implement the missing-macro behavior for include strings to pass the test.
- [x] 5.16 **RED**: Add a failing test verifying that for preset file version 7/8, any `${...}` in an include string marks the file node Unresolved with reason `UnsupportedMacro`.
- [x] 5.17 **GREEN**: Implement the v7/v8 include macro restriction to pass the test.

## 6. preset-inheritance-graph

- [x] 6.1 **RED**: In a new `tests/PresetsGraph/InheritanceGraphTests.cpp`, write a failing test for adding a `PresetPayload`.
- [x] 6.2 **GREEN**: Implement `PresetInheritanceGraph` and node addition to pass the test.
- [x] 6.3 **RED**: Add a failing test for tracking a preset's availability as `Active` when its condition evaluates to true and it is not hidden.
- [x] 6.4 **GREEN**: Implement the condition evaluation logic to pass the test.
- [x] 6.5 **RED**: Add a failing test for resolving an inheritance link between two presets.
- [x] 6.6 **GREEN**: Implement the inheritance resolution logic to pass the test.
- [x] 6.7 **RED**: Add a failing test for computing an `Unresolved` state when a condition cannot be evaluated.
- [x] 6.8 **GREEN**: Implement the graph state computation logic to pass the test.
- [x] 6.8a **RED**: Add a failing test verifying that a hidden preset is reported as Hidden.
- [x] 6.8b **GREEN**: Implement hidden availability handling.
- [x] 6.8c **RED**: Add a failing test verifying that a preset containing `$vendor{...}` is reported as Disabled.
- [x] 6.8d **GREEN**: Implement vendor-macro disable handling.
- [x] 6.9 **RED**: Add a failing test verifying that a cyclic `inherits` relationship is detected and marks affected presets as Unresolved with reason `InheritanceCycle`.
- [x] 6.10 **GREEN**: Implement inheritance cycle detection to pass the test.
- [x] 6.11 **REFACTOR**: Review `PresetInheritanceGraph`.

## 6a. preset-model

- [x] 6a.1 **RED**: In a new `tests/PresetsGraph/PresetModelTests.cpp`, write a failing test that a preset can be tagged as Configure/Build/Test/Package/Workflow based on which array it came from.
- [x] 6a.2 **GREEN**: Implement minimal preset model types to pass the test.
- [x] 6a.3 **RED**: Add a failing test that inheritance precedence prefers earlier entries in `inherits` for scalar conflicts.
- [x] 6a.4 **GREEN**: Implement scalar merge precedence to pass the test.
- [x] 6a.5 **RED**: Add a failing test that environment merge prefers earlier `inherits` entries and supports `null` to remove inherited keys.
- [x] 6a.6 **GREEN**: Implement environment merge behavior to pass the test.
- [x] 6a.6a **RED**: Add a failing test verifying that for a Build/Test/Package preset, `inheritConfigureEnvironment` merges the associated configure environment between inherited and explicit preset environment.
- [x] 6a.6b **GREEN**: Implement `inheritConfigureEnvironment` merge behavior to pass the test.
- [x] 6a.7 **RED**: Add a failing test that environment values can reference each other out of order using `$env{}`.
- [x] 6a.8 **GREEN**: Implement environment expansion to pass the test.
- [x] 6a.9 **RED**: Add a failing test that environment reference cycles are detected.
- [x] 6a.10 **GREEN**: Implement cycle detection and mark Unresolved with reason `EnvironmentCycle`.
- [x] 6a.11 **RED**: Add a failing test verifying that inherited values expand using the active preset's `${presetName}`.
- [x] 6a.12 **GREEN**: Implement preset-associated macro injection to pass the test.
- [x] 6a.13 **RED**: Add a failing test verifying that `${generator}` is populated for a ConfigurePreset with an effective generator.
- [x] 6a.14 **GREEN**: Implement configure preset generator injection to pass the test.
- [x] 6a.15 **RED**: Add a failing test verifying that `${generator}` for a Build/Test/Package preset is derived from its `configurePreset`.
- [x] 6a.16 **GREEN**: Implement generator derivation via `configurePreset` to pass the test.
- [x] 6a.17 **REFACTOR**: Review preset model and inheritance resolution.

## 7. preset-graph-manager

- [x] 7.1 **RED**: In a new `tests/PresetsGraph/GraphManagerTests.cpp`, write a failing test verifying that an `ApplyContext` call that discovers a new file results in a new node in the include graph.
- [x] 7.2 **GREEN**: Implement the `PresetsGraph` manager and the basic resolution loop to pass the test.
- [x] 7.2a **RED**: Add a failing test verifying that the manager uses the file loader to load a newly discovered include file and parses it with `nlohmann::json::parse(...)`.
- [x] 7.2b **GREEN**: Implement the file loading + JSON parsing behavior to pass the test.
- [x] 7.2c **RED**: Add a failing test verifying that a file loader "FileDoesNotExist" failure causes the corresponding file node to be marked Unresolved with reason `FileDoesNotExist` and that the manager state is Unresolved.
- [x] 7.2d **GREEN**: Implement failure handling to pass the test.
- [x] 7.2e **RED**: Add a failing test verifying that invalid JSON causes the corresponding file node to be marked Unresolved with reason `InvalidJson` and that the manager state is Unresolved.
- [x] 7.2f **GREEN**: Implement invalid JSON handling to pass the test.
- [x] 7.2g **RED**: Add a failing test verifying that a relative include path is resolved relative to the including preset file path.
- [x] 7.2h **GREEN**: Implement relative include path resolution to pass the test.
- [x] 7.2i **RED**: Add a failing test verifying that `${fileDir}` is populated for include resolution and expands correctly.
- [x] 7.2j **GREEN**: Implement `${fileDir}` injection for include resolution to pass the test.
- [x] 7.2k **RED**: Add a failing test verifying that `${dollar}` is populated and expands to a literal `$`.
- [x] 7.2l **GREEN**: Implement `${dollar}` injection to pass the test.
- [x] 7.2m **RED**: Add a failing test verifying that when simulated CMake is too old for `cmakeMinimumRequired`, the file node is marked Unresolved with reason `CMakeMinimumRequiredNotMet`.
- [x] 7.2n **GREEN**: Implement `cmakeMinimumRequired` enforcement to pass the test.
- [x] 7.2o **RED**: Add a failing test verifying that when a file declares a preset file `version` unsupported by the simulated CMake version, the file node is marked Unresolved with reason `PresetVersionUnsupported`.
- [x] 7.2p **GREEN**: Implement preset file version support validation to pass the test.
- [x] 7.2q **RED**: Add a failing test verifying that when a file has no root `version` field, the file node is marked Unresolved with reason `PresetVersionMissing` and its includes are not processed.
- [x] 7.2r **GREEN**: Implement missing-version handling to pass the test.
- [x] 7.2s **RED**: Add a failing test verifying that when a file declares `version` < 4 and specifies `include`, the file node is marked Unresolved with reason `IncludeFieldUnsupportedInPresetVersion`.
- [x] 7.2t **GREEN**: Implement include-field version enforcement to pass the test.
- [x] 7.3 **RED**: Add a failing test that constructs a cyclic include dependency that the manager must detect.
- [x] 7.4 **GREEN**: Implement the include cycle detection within the resolution loop and mark Unresolved with reason `IncludeCycle`.
- [x] 7.5 **RED**: Add a failing test verifying the composite `Unresolved` state when the include graph is `Resolved` but the inheritance graph is `Unresolved`.
- [x] 7.6 **GREEN**: Implement the composite state logic to pass the test.
- [x] 7.7 **REFACTOR**: Review `PresetsGraph` manager.
