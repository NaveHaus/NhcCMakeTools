## 1. Supported Preset Version Boundary (RED -> GREEN -> REFACTOR)

- [ ] 1.1 RED: Add a failing `GraphManagerTests.cpp` scenario that a simulated CMake 4.2 manager reports preset file version 10.
- [ ] 1.2 GREEN: Implement the minimal supported-version mapping update so simulated CMake 4.2 no longer reports preset file version 11.
- [ ] 1.3 RED: Add a failing `GraphManagerTests.cpp` scenario that a simulated CMake 4.3 manager reports preset file version 11.
- [ ] 1.4 GREEN: Implement the minimal boundary logic so preset file version 11 is reported only for simulated CMake 4.3 and newer.
- [ ] 1.5 REFACTOR: Review the version-mapping branch order for clarity without changing the specified behavior.

## 2. Reapply Context State Recovery (RED -> GREEN -> REFACTOR)

- [ ] 2.1 RED: Add a failing `GraphManagerTests.cpp` scenario where one `ApplyContext()` call leaves a file node unresolved as missing, a later `ApplyContext()` reload succeeds, and the stale `FileDoesNotExist` reason is no longer present.
- [ ] 2.2 GREEN: Implement the minimal manager-side reset of file-node unresolved load state before a reload attempt to satisfy the recovery test.
- [ ] 2.3 REFACTOR: Isolate any repeated file-node reset logic so reload behavior remains testable and localized to the manager load path.
- [ ] 2.4 RED: Add a failing `GraphManagerTests.cpp` scenario where a first `ApplyContext()` call marks a file node Unresolved with reason `ParseError`, the file is then removed, a later `ApplyContext()` still fails, and the node is left Unresolved with reason `FileDoesNotExist` — not the stale `ParseError`.
- [ ] 2.5 GREEN: Confirm the fresh-reason assignment for failed reloads is satisfied by the existing failure paths that run after the reset introduced in 2.2, with no additional implementation required.

## 3. Verification (RED -> GREEN -> REFACTOR)

- [ ] 3.1 RED: Confirm the new and updated `GraphManagerTests` scenarios fail for the expected reasons before the implementation is finalized.
- [ ] 3.2 GREEN: Run the targeted `GraphManagerTests` build and test commands until the new scenarios pass.
- [ ] 3.3 REFACTOR: Run the appropriate workflow preset for full project verification after the focused tests pass.
