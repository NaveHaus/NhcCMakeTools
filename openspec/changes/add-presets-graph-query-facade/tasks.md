## 1. Readiness

- [ ] 1.1 Confirm changes `fix-include-resolution-policy`, `fix-effective-condition-availability`, `fix-manager-version-and-reapply-state`, and `complete-resolved-state-model` are implemented or otherwise available as the behavioral foundation for this facade
- [ ] 1.2 Review the current `PresetsGraph`, `PresetModel`, include-graph, and inheritance-graph public APIs to identify the minimum data projections required for the facade

## 2. Facade Contract Tests

- [ ] 2.1 **RED**: Add a failing test covering manager-provided access to a high-level query facade after `ApplyContext(...)`
- [ ] 2.2 **RED**: Add a failing test covering typed preset summary lookup, including preset type, source file path, availability, unresolved reason, and workflow diagnostics
- [ ] 2.3 **RED**: Add a failing test covering resolved-field queries that preserve unresolved, partially resolved, and fully resolved field status from the preset model
- [ ] 2.4 **RED**: Add failing tests covering common relationship queries for file includes, file-declared presets, preset parents, and preset children

## 3. Facade Implementation

- [ ] 3.1 **GREEN**: Implement the read-only query facade type and bind it to manager-owned applied state rather than a separate cache or resolver
- [ ] 3.2 **GREEN**: Implement manager-level access to the facade as the preferred high-level query entry point while preserving direct low-level APIs
- [ ] 3.3 **GREEN**: Implement typed preset summary projections using existing preset-model, inheritance-graph, and workflow-diagnostic state
- [ ] 3.4 **GREEN**: Implement resolved-field queries as direct projections of preset-model resolved state
- [ ] 3.5 **GREEN**: Implement common file and preset relationship queries using existing include-graph and inheritance-graph state

## 4. Refactor And Verification

- [ ] 4.1 **REFACTOR**: Review the facade API for accidental duplication of resolution logic and remove any behavior that belongs in lower layers
- [ ] 4.2 **REFACTOR**: Tighten naming and type boundaries so facade queries stay explicit about preset type and current-state ownership
- [ ] 4.3 Run the focused preset-graph tests added for this change
- [ ] 4.4 Run `cmake --workflow --preset=clangd-ninja-vcpkg-release-test` to verify the full build and test workflow after the facade is implemented
