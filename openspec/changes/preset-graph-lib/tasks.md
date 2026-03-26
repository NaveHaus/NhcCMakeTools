## 1. Setup

- [ ] 1.1 Add CMake target for a static library `NhcPresetGraph`
- [ ] 1.2 Create test directory `tests/PresetsGraph`
- [ ] 1.3 Add `nlohmann/json` dependency for `NhcPresetGraph`
- [ ] 1.4 Add a file loader abstraction for reading preset files as strings

## 2. preset-macro-context

- [ ] 2.1 **RED**: In a new `tests/PresetsGraph/MacroContextTests.cpp`, write a failing test for storing and retrieving a macro value.
- [ ] 2.2 **GREEN**: Implement the `MacroContext` class and minimal code to pass the test.
- [ ] 2.3 **RED**: Add a failing test for expanding a string with a known macro.
- [ ] 2.4 **GREEN**: Implement the expansion logic to pass the test.
- [ ] 2.5 **RED**: Add a failing test for gracefully handling a string with an unknown macro (partial expansion).
- [ ] 2.6 **GREEN**: Implement the partial expansion logic to pass the test.
- [ ] 2.7 **RED**: Add a failing test verifying that `$env{NAME}` prefers the preset environment value over the parent environment value.
- [ ] 2.8 **GREEN**: Implement `$env{}` expansion logic to pass the test.
- [ ] 2.9 **RED**: Add a failing test verifying that `$penv{NAME}` uses only the parent environment value.
- [ ] 2.10 **GREEN**: Implement `$penv{}` expansion logic to pass the test.
- [ ] 2.10a **RED**: Add a failing test verifying that when the MacroContext parent environment map does not contain a variable, `$penv{VAR}` remains unresolved (not replaced with an empty string).
- [ ] 2.10b **GREEN**: Implement the missing-parent-variable behavior to pass the test.
- [ ] 2.10c **RED**: Add a failing test verifying that when neither preset nor parent environment defines a variable, `$env{VAR}` remains unresolved (not replaced with an empty string).
- [ ] 2.10d **GREEN**: Implement the missing-variable behavior to pass the test.
- [ ] 2.11 **REFACTOR**: Review `MacroContext` for clarity and correctness.

## 3. preset-graph-core

- [ ] 3.1 **RED**: In a new `tests/PresetsGraph/CoreGraphTests.cpp`, write a failing test for adding a node payload and receiving a unique ID.
- [ ] 3.2 **GREEN**: Implement the base `Graph` class with node addition logic to pass the test.
- [ ] 3.3 **RED**: Add a failing test for creating a directed edge between two nodes.
- [ ] 3.4 **GREEN**: Implement edge tracking logic to pass the test.
- [ ] 3.5 **RED**: Add a failing test that asserts a cycle is detected when a cyclic edge is added.
- [ ] 3.6 **GREEN**: Implement cycle detection logic to pass the test.
- [ ] 3.7 **REFACTOR**: Review core graph structures.

## 4. preset-condition-ast

- [ ] 4.1 **RED**: In a new `tests/PresetsGraph/ConditionTests.cpp`, write a failing test for a simple `EqualsCondition` that should evaluate to true.
- [ ] 4.2 **GREEN**: Implement the `Condition` interface and `EqualsCondition` class to pass the test.
- [ ] 4.3 **RED**: Add a failing test for a condition that cannot be evaluated due to a missing macro.
- [ ] 4.4 **GREEN**: Implement the logic to return an `Unknown` or indeterminate state to pass the test.
- [ ] 4.5 **RED**: Add a failing test for a logical `AllOfCondition`.
- [ ] 4.6 **GREEN**: Implement the `AllOfCondition` class to pass the test.
- [ ] 4.7 **REFACTOR**: Review `Condition` AST classes.

## 5. preset-include-graph

- [ ] 5.1 **RED**: In a new `tests/PresetsGraph/IncludeGraphTests.cpp`, write a failing test for adding a `FilePayload`.
- [ ] 5.2 **GREEN**: Implement the `PresetIncludeGraph` class and node addition to pass the test.
- [ ] 5.3 **RED**: Add a failing test for computing an `Unresolved` state when a node has a pending include.
- [ ] 5.4 **GREEN**: Implement the state computation logic to pass the test.
- [ ] 5.5 **RED**: Add a failing test for resolving a pending include into an edge when the macro context is sufficient.
- [ ] 5.6 **GREEN**: Implement the include resolution logic to pass the test.
- [ ] 5.7 **REFACTOR**: Review `PresetIncludeGraph`.
- [ ] 5.8 **RED**: Add a failing test for marking a file node as Unresolved with reason `FileDoesNotExist`.
- [ ] 5.9 **GREEN**: Implement the unresolved-reason tracking to pass the test.

## 6. preset-inheritance-graph

- [ ] 6.1 **RED**: In a new `tests/PresetsGraph/InheritanceGraphTests.cpp`, write a failing test for adding a `PresetPayload`.
- [ ] 6.2 **GREEN**: Implement `PresetInheritanceGraph` and node addition to pass the test.
- [ ] 6.3 **RED**: Add a failing test for tracking a preset's status as `Enabled` when its condition evaluates to true.
- [ ] 6.4 **GREEN**: Implement the condition evaluation logic to pass the test.
- [ ] 6.5 **RED**: Add a failing test for resolving an inheritance link between two presets.
- [ ] 6.6 **GREEN**: Implement the inheritance resolution logic to pass the test.
- [ ] 6.7 **RED**: Add a failing test for computing an `Unresolved` state when a condition cannot be evaluated.
- [ ] 6.8 **GREEN**: Implement the graph state computation logic to pass the test.
- [ ] 6.9 **REFACTOR**: Review `PresetInheritanceGraph`.

## 7. preset-graph-manager

- [ ] 7.1 **RED**: In a new `tests/PresetsGraph/GraphManagerTests.cpp`, write a failing test verifying that an `ApplyContext` call that discovers a new file results in a new node in the include graph.
- [ ] 7.2 **GREEN**: Implement the `PresetsGraph` manager and the basic resolution loop to pass the test.
- [ ] 7.2a **RED**: Add a failing test verifying that the manager uses the file loader to load a newly discovered include file and parses it with `nlohmann::json::parse(...)`.
- [ ] 7.2b **GREEN**: Implement the file loading + JSON parsing behavior to pass the test.
- [ ] 7.2c **RED**: Add a failing test verifying that a file loader "FileDoesNotExist" failure causes the corresponding file node to be marked Unresolved with reason `FileDoesNotExist` and that the manager state is Unresolved.
- [ ] 7.2d **GREEN**: Implement failure handling to pass the test.
- [ ] 7.2e **RED**: Add a failing test verifying that invalid JSON causes the corresponding file node to be marked Unresolved with reason `InvalidJson` and that the manager state is Unresolved.
- [ ] 7.2f **GREEN**: Implement invalid JSON handling to pass the test.
- [ ] 7.2g **RED**: Add a failing test verifying that a relative include path is resolved relative to the including preset file path.
- [ ] 7.2h **GREEN**: Implement relative include path resolution to pass the test.
- [ ] 7.3 **RED**: Add a failing test that constructs a cyclic include dependency that the manager must detect.
- [ ] 7.4 **GREEN**: Implement the cycle detection within the resolution loop to pass the test.
- [ ] 7.5 **RED**: Add a failing test verifying the composite `Unresolved` state when the include graph is `Resolved` but the inheritance graph is `Unresolved`.
- [ ] 7.6 **GREEN**: Implement the composite state logic to pass the test.
- [ ] 7.7 **REFACTOR**: Review `PresetsGraph` manager.
