## Context

The project is set up to enable CTest-based unit tests when `BUILD_TESTING=ON` (top-level `CMakeLists.txt`). The repo uses vcpkg and already depends on Catch2 (`vcpkg.json`).

Current state (as of this change proposal):

- `src/TestLib/CMakeLists.txt` and `tests/CMakeLists.txt` are minimal stubs that call `find_package(Catch2 ...)`.
- Test executables are expected to be created via `nhc_add_test_executable()` (defined in `cmake/NhcTargetFunctions.cmake`).
- The intended convention is to link tests with `USES NhcTestLib` (documented in `AGENTS.md`).

Constraint: `NhcTestLib` must only exist when `BUILD_TESTING=ON`, and it must be an explicitly static library.

## Goals / Non-Goals

**Goals:**

- Provide a single static library target `NhcTestLib` under `src/TestLib`.
- Centralize Catch2 integration so tests do not need to duplicate `find_package(Catch2 ...)`.
- Provide a project-owned `main(int, char**)` for all test executables.
- Ensure `NhcTestLib` is only defined when `BUILD_TESTING=ON`.

**Non-Goals:**

- Adding extensive shared test utilities/fixtures beyond what is needed to establish the harness.
- Changing production/library code behavior.
- Introducing additional test frameworks.

## Decisions

- **Provide our own `main()` in NhcTestLib (chosen)**
  - Rationale: gives a single stable entry point that can later be extended (environment setup, global reporting, etc.) without touching each test target.
  - Alternative: link `Catch2::Catch2WithMain` and use an INTERFACE `NhcTestLib` (simpler, but does not satisfy the explicit “project-owned main” requirement and limits future customization).

- **`NhcTestLib` is `STATIC` (chosen)**
  - Rationale: matches requirement; also avoids needing special handling for runtime artifacts.
  - Alternative: `OBJECT` library can guarantee object inclusion semantics, but conflicts with the explicit static requirement.

- **`NhcTestLib` links to `Catch2::Catch2` with PUBLIC usage requirements (chosen)**
  - Rationale: tests include Catch2 headers; linking `Catch2::Catch2` as PUBLIC ensures include dirs/compile definitions propagate to all test executables.
  - Alternative: tests each link Catch2 directly; rejected because it duplicates integration and makes global adjustments harder.

- **Locate and gate Catch2 discovery inside `src/TestLib` (chosen)**
  - Rationale: makes `src/TestLib` the single integration point; combined with `BUILD_TESTING` gating, avoids requiring Catch2 for non-test builds.
  - Alternative: keep `find_package(Catch2 ...)` in `tests/CMakeLists.txt`; acceptable but spreads ownership.

### CMake Wiring (intended)

High-level relationship:

```
top-level CMakeLists.txt
  add_subdirectory(src)
    (when BUILD_TESTING=ON) add_subdirectory(TestLib) -> defines NhcTestLib
  (when BUILD_TESTING=ON) add_subdirectory(tests)
    nhc_add_test_executable(FooTests ... USES NhcTestLib)
```

`src/TestLib/CMakeLists.txt` should:

- `find_package(Catch2 3.11 REQUIRED)`
- define `NhcTestLib` as STATIC with a single translation unit that defines `main()`.
- `target_link_libraries(NhcTestLib PUBLIC Catch2::Catch2)`
- apply the project’s standard target defaults (either via `nhc_configure_library()` or via `nhc_add_library` if it is extended to support PUBLIC/PRIVATE usage requirements).

Test executables should:

- never define their own `main()` (no `CATCH_CONFIG_MAIN`, no `CATCH_CONFIG_RUNNER`).
- link `NhcTestLib` (directly, or by specifying `USES NhcTestLib` when calling `nhc_add_test_executable`).

## Risks / Trade-offs

- **[Risk] Multiple `main()` definitions** → Mitigation: document “tests must not define `main()`”; optionally add a lightweight CI check later (e.g., grep for `CATCH_CONFIG_MAIN` in `tests/`).
- **[Risk] Catch2 usage requirements not propagating** → Mitigation: ensure the link to `Catch2::Catch2` is PUBLIC (not PRIVATE) so includes/defines are inherited by tests.
- **[Trade-off] Central entry point becomes a choke point** → Mitigation: keep `main()` minimal initially (just `Catch::Session().run(...)`) and expand only when justified.
