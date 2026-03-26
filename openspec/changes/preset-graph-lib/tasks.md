## 1. Setup

- [ ] 1.1 Add CMake target for a static library `NhcPresetGraph`
- [ ] 1.2 Create test directory `tests/PresetsGraph`

## 2. preset-macro-context

- [ ] 2.1 **RED**: In a new `tests/PresetsGraph/MacroContextTests.cpp`, write a failing test for storing and retrieving a macro value.
- [ ] 2.2 **GREEN**: Implement the `MacroContext` class and minimal code to pass the test.
- [ ] 2.3 **RED**: Add a failing test for expanding a string with a known macro.
- [ ] 2.4 **GREEN**: Implement the expansion logic to pass the test.
- [ ] 2.5 **RED**: Add a failing test for gracefully handling a string with an unknown macro (partial expansion).
- [ ] 2.6 **GREEN**: Implement the partial expansion logic to pass the test.
- [ ] 2.7 **REFACTOR**: Review `MacroContext` for clarity and correctness.

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
- [ ] 7.3 **RED**: Add a failing test that constructs a cyclic include dependency that the manager must detect.
- [ ] 7.4 **GREEN**: Implement the cycle detection within the resolution loop to pass the test.
- [ ] 7.5 **RED**: Add a failing test verifying the composite `Unresolved` state when the include graph is `Resolved` but the inheritance graph is `Unresolved`.
- [ ] 7.6 **GREEN**: Implement the composite state logic to pass the test.
- [ ] 7.7 **REFACTOR**: Review `PresetsGraph` manager.
