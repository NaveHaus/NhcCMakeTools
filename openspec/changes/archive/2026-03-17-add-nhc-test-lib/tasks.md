## 1. CMake Integration

- [x] 1.1 Update `src/CMakeLists.txt` so `src/TestLib` is only added when `BUILD_TESTING=ON`
- [x] 1.2 Define `NhcTestLib` as an explicit `STATIC` library target in `src/TestLib/CMakeLists.txt`
- [x] 1.3 Ensure `src/TestLib/CMakeLists.txt` owns `find_package(Catch2 3.11 REQUIRED)` and links `Catch2::Catch2` to `NhcTestLib` with PUBLIC usage requirements

## 2. Test Entry Point

- [x] 2.1 Add a single translation unit under `src/TestLib/` that implements `main(int, char**)` using Catch2 (runner/session)
- [x] 2.2 Ensure the project builds without duplicate `main()` when tests link `NhcTestLib` (no `CATCH_CONFIG_MAIN` in test sources)

## 3. Test Consumption Pattern

- [x] 3.1 Refactor `tests/CMakeLists.txt` to stop directly depending on Catch2 and instead define tests that link `NhcTestLib`
- [x] 3.2 Add a minimal smoke test executable (new `tests/TestLib/*Tests.cpp`) that uses Catch2 TDD style and links `NhcTestLib`
- [x] 3.3 Register the smoke test with CMake using `nhc_add_test_executable(... USES NhcTestLib)`

## 4. Verification

- [x] 4.1 Configure/build/test with the standard workflow preset (`cmake --workflow --preset=vs18-vcpkg-mt-s-release-test`)
- [x] 4.2 Confirm configuring with `BUILD_TESTING=OFF` does not require Catch2 to be installed/discoverable
