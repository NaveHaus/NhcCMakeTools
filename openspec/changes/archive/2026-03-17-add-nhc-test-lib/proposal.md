## Why

The repo needs a single shared test support library so all unit tests use a consistent Catch2 configuration and a single, project-owned `main()`.
This removes per-test boilerplate and makes it easier to evolve test startup behavior over time.

## What Changes

- Add a static library target `NhcTestLib` under `src/TestLib`.
- `NhcTestLib` links `Catch2::Catch2` and provides the `main()` entry point used by all test executables.
- Ensure `NhcTestLib` is only defined when `BUILD_TESTING=ON`.
- Update test CMake patterns so test executables link `NhcTestLib` (directly or via `nhc_add_test_executable(... USES NhcTestLib)`).
- Remove redundant `find_package(Catch2 ...)` usage from test executables once `NhcTestLib` is the single integration point.

## Capabilities

### New Capabilities

- `nhc-test-lib`: A shared test harness library that centralizes Catch2 integration and provides the single `main()` for all tests.

### Modified Capabilities

<!-- None -->

## Impact

- CMake: `src/TestLib/CMakeLists.txt`, `tests/**/CMakeLists.txt` and potentially the top-level `tests/CMakeLists.txt` to rely on `NhcTestLib` rather than duplicating Catch2 setup.
- Dependencies: `Catch2` remains a vcpkg dependency, but should only be required/located when `BUILD_TESTING=ON`.
- Developer workflow: all new tests must link `NhcTestLib` and should not define their own `main()`.
