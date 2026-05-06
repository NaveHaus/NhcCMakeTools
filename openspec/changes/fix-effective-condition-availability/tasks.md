## 1. Preset-model condition resolution

- [ ] 1.1 **RED**: Add a failing `tests/PresetsGraph/PresetModelTests.cpp` scenario that calls effective-condition resolution on a direct inherits cycle and asserts the lookup returns without unbounded recursion.
- [ ] 1.2 **GREEN**: Add cycle protection to `PresetModel::ResolveCondition()` while preserving current local-condition and explicit-`null` semantics.
- [ ] 1.3 **RED**: Add a failing `tests/PresetsGraph/PresetModelTests.cpp` scenario that verifies a descendant does not inherit an ancestor condition through an intermediate preset with `condition: null`.
- [ ] 1.4 **GREEN**: Update effective-condition traversal so explicit `condition: null` clears the current preset while remaining non-inheritable to descendants.
- [ ] 1.5 **REFACTOR**: Review the condition-resolution helper structure and naming so cycle-safe lookup remains the single source of truth for effective-condition semantics.

## 2. Inheritance availability refresh

- [ ] 2.1 **RED**: Add a failing `tests/PresetsGraph/GraphManagerTests.cpp` or `tests/PresetsGraph/InheritanceGraphTests.cpp` scenario where a child preset without a local condition inherits `condition: false` and is incorrectly reported Active today.
- [ ] 2.2 **GREEN**: Update inheritance-graph refresh to publish the effective inherited condition from `PresetModel::ResolveCondition()` into each preset payload before availability evaluation.
- [ ] 2.3 **RED**: Add a failing availability scenario that verifies a preset with effective explicit `null` remains Active after the refresh-path change.
- [ ] 2.4 **GREEN**: Confirm availability evaluation continues to treat absent or explicit-null effective conditions as Active while inherited false conditions disable descendants.
- [ ] 2.5 **REFACTOR**: Remove any duplicated condition-resolution logic from the refresh path so availability depends only on the payload supplied by the preset model.

## 3. Verification

- [ ] 3.1 Reconfigure the affected build preset if test registration or CMake inputs change.
- [ ] 3.2 Build and run the targeted `PresetModel` and inheritance/graph test executables for the RED/GREEN cycle.
- [ ] 3.3 Run the required workflow verification preset and confirm the full preset-graph test suite passes with the new effective-condition behavior.
- [ ] 3.4 **REGRESSION**: Add a scenario exercising a cyclic `inherits` graph after effective-condition publishing and confirm the Inheritance Graph still reports `InheritanceCycle` for affected presets — cycle-safe condition lookup MUST NOT suppress or replace that diagnostic.
