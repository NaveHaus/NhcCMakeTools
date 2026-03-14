# Context, Rules, and Guidelines for AI Agents
## Tech stack
- c++23
- clang-format
- CMake 3.31+
- Qt 6.9
- Catch2 v3.11+
- Doxygen 1.16+
- GitHub Flavored Markdown

# Agent Guidelines
The NhcVckgTools project hosts tools that focus on improving quality-of-life for developers working with the [CMake](https://cmake.org) cross-platform build system.

## Safety & Operational Rules (MANDATORY)
To maintain repository integrity, agents MUST follow these rules:
- **No Secrets**: NEVER commit secrets, API keys, or credentials.
- **Git Safety**:
  - DO NOT modify git configuration.
  - DO NOT use `--no-verify` or bypass hooks unless explicitly requested.
  - DO NOT force push to protected branches, e.g. `master` or `main`.
  - DO NOT automatically fix `git` errors---ALWAYS ask the user for confirmation first.
  - DO warn the user if attempting to commit to `master` or `main`.
  - DO warn the user if `git` returns an error or reports that the remote branch is missing.
  - DO offer to resolve `git` errors, presenting the user with 1-4 options for doing so.
- **Path Handling**: Always use absolute paths when interacting with file system tools.
- **Verification**: Always run build and test commands after modifications.

## Terminology
- **openspec**: An artifact-driven workflow for managing software changes (features, fixes, etc.) through structured specifications and tasks.
- **opsx-***: Shortcut commands for interacting with the OpenSpec workflow.
- **nhc-opsx-commit**: A helper command that uses OpenSpec artifacts and the `conventional-commits` skill to generate compliant git commit messages.

## Process
### Requirements (MANDATORY)
- An `openspec` workflow MUST be used to plan new features and modifications of existing features (e.g. bug fixes). Warn the user if the `openspec` directory is missing or inaccessible.
- A test-driven development (TDD) RED/GREEN/REFACTORING workflow is required when implementing ALL code additions and modifications. Warn the user if the `tdd` skill is missing or inaccessible.
- The `nhc-opsx-commit` command MUST be used to generate a `git` commit message and commit changes made WITH an `openspec` workflow. Warn the user if the `nhc-opsx-commit` command is missing or inaccessible.
- The `conventional-commits` skill MUST be used to generate a `git` commit message for changes made WITHOUT an `openspec` workflow. Warn the user if the `conventional-commits` skill is missing or inaccessible.
- NOTE: If unsure about which commit strategy applies, you MUST ask the user to avoid generating spurious or erroneous commits.

Most common sequence of `openspec` operations:
1. `opsx-new <name>`: Initialize the change.
2. `opsx-ff <name>`: Generate/update artifacts.
3. `opsx-apply <name>`: Implement the tasks (following TDD process - see [Testing](#testing)).
4. `./build.ps1 -Tasks test`: Verify all tests before completing the change.
5. `opsx-verify <name>`: Validate implementation against specs (see [Error Handling Guidance](#error-handling-guidance) if this fails).
6. `git add ./openspec/ <paths-to-changed-files-and-directories>`: Prepare to commit the changes.
7. `nhc-opsx-commit`: Generate a `conventional-commits` `git` commit message based on the changes and complete the commit.
8. `opsx-sync <name>`: Make change specs permanent.
9. `opsx-archive <name>`: Archive the change.
10. `git add ./openspec/`: Prepare to commit the archived change artifacts.
11. `nhc-opsx-commit`: Generate a `conventional-commits` `git` commit message based on the changes and complete the commit.

## Code Style
### Requirements (MANDATORY)
- DO use camel-case for class names, e.g. `class MyNewClass`.
- DO use `<Class>.h`/`<Class>.cpp` when implementing new classes, where `<Class>` is the exact, camel-case name of the class to be implemented, e.g. `MyNewClass.h`/`MyNewClass.cpp`.
- DO use `m_<Member>` for class member variables, where `<Member>` is a camel-case variable name, e.g. `m_EnableDebug`.
- DO use `Get<Member>()`/`Set<Member>()` for setter-getter class methods that access a member variable, where `<Member>` is the `m_<Member>` member variable.
- DO use `<Member><Property>()` for pure observers that cannot be directly modified by a caller, e.g. `ItemCount()`.
- Do use `<Action><Property>(<args>)` for updating properties that can be counted, e.g. `AddItem(<args>)`/`RemoveItem(<args>)`/`SetItem(<args>)`.
- DO run `clang-format -i <file>.cpp` after modifying C++ source code to ensure the code is formatted properly.
- DO use C++-style Doxygen comments for developer-facing class documentation, e.g.:
  ```cpp
  /// Base class for all FibbetyGibbet managers.
  ///
  /// ... More detailed explanation of the class behavior and general usage requirements ...
  class AbstractFibbetyGibbetManager {
    protected:
      /// Default constructor.
      ///
      /// Has no side-effects.
      AbstractFibbetyGibbetManager();

      /// Virtual destructor.
      ///
      /// Has no side-effects.
      virtual ~AbstractFibbetyGibbetManager();

    public:
      /// Retrieves the number of fibbeties.
      ///
      /// Must be overridden by derived classes.
      /// @returns The number of fibbeties managed by this instance.
      virtual unsigned FibbetyCount() = 0;

      /// Adds a new gibbety to this instance.
      ///
      /// Must be overridden by derived classes.
      virtual void AddGibbety() = 0;

      ... rest of class implementation ...
  };
  ```

## Building and Testing
### Requirements (MANDATORY)
- A test-driven development (TDD) process MUST be used when changing or adding code.
- ALL changes to existing code MUST be tested by updating ALL relevant existing tests following TDD.
- Tests MUST be stored under `tests/<category>`, where `<category>` MUST be the name of the library or tool containing the feature/class under test.
- Test files MUST follow the naming convention: `tests/<category>/<feature>Tests.cpp`, e.g. `tests/NhcMyLibName/MyClassTests.cpp` for a library class test, or `tests/NhcMyToolName/MyToolSpecificClassTests.cpp` for an executable tool.
  - `<feature>` will normally be the name of a C++ class under test, e.g. `MyClassTests.cpp`
  - The test must be registered with CMake by modifying `tests/<category>/CMakeLists.txt` to call the `nhc_add_test_executable()` CMake function, defined in `cmake/NhcTargetFunctions.cmake`, e.g.:
    ```cmake
    nhc_add_test_executable(<feature>Tests SOURCES <feature>Tess.cpp USES NhcTestLib)
    ```
    Note: `USES NhcTestLib` ensures Catch2 is configured properly for the test.
- [Catch2](https://github.com/catchorg/Catch2/blob/v3.11.0/docs/test-cases-and-sections.md#) behavior-driven (BDD) testing MUST be used to implement tests aligned with specification of the functionality to be implemented, e.g.:
  ```cpp
  SCENARIO("vector can be sized and resized") {
    GIVEN("An empty vector") {
      auto v = std::vector<std::string>{};

      // Validate assumption of the GIVEN clause
      THEN("The size and capacity start at 0") {
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
      }

      // Validate one use case for the GIVEN object
      WHEN("push_back() is called") {
        v.push_back("hullo");

        THEN("The size changes") {
            REQUIRE(v.size() == 1);
            REQUIRE(v.capacity() >= 1);
        }
      }
    }
  }
  ```
  Note: If you think a BDD test cannot be used or does not apply, iterate with the user to come up with a workable solution.

- If modifications are made to any `CMakeLists.txt`, execute `cmake --workflow --preset=<preset>` to reconfigure and execute the build to check for errors, e.g. `cmake --workflow --preset=vs18-vcpkg-mt-s-release` to reconfigure and execute a Visual Studio 18 2026 build.
  - Note: If the preset to use is unclear, ask the user.
- Use `cmake --build --preset=<preset>` to build code after changes are made to source to check for errors, e.g. `cmake --build --preset=vs18-vcpkg-mt-s-release` to execute a Visual Studio 18 2026 build.
  - Note: If the preset to use is unclear, ask the user.
- ALL tests for EACH change MUST be verified once a the change is complete by running `cmake --workflow --preset=<preset> ; ctest --preset=<preset> --tests-regex '<feature>Tests'`, e.g. to build and run `MyClassTests` for Visual Studio 18 2026:
  ```powershell
  cmake --build --preset=vs18-vcpkg-mt-s-release
  ctest --preset=vs18-vcpkg-mt-s-release -R 'MyClassTests'
  ```
  - Note: If the preset to use is unclear, ask the user.
- ALL tests MUST be verified once all changes are complete by running `cmake --workflow --preset=<preset>-test`, where `<preset>` corresponds to the current build under test, e.g. `cmake --workflow --preset=vs18-vcpkg-mt-s-release-test` to build and run the Visual Studio 18 2026 tests.
  Note: If the preset to use is unclear, ask the user.

## Example Openspec Workflows
### One-Shot Implementation
Can be used for simple changes that require no investigation or decision making prior to implementation:
- Generate the `openspec` change artifacts in one shot:
  - `opsx-new <change-name>`
  - `opsx-ff <change-name>`
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - `./build.ps1 -Tasks test`
  - `opsx-verify <change-name>`
- Locally commit the working `openspec` artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`; e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - `nhc-opsx-commit`
- Locally archive the `openspec` completed change artifacts:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
  - `git add ./openspec/`
  - `nhc-opsx-commit`
### One-Shot Exploration to Implementation
Can be used for straightforward changes that require some upfront investigation and/or decision making prior to implementing.
- Interactively research and investigate a change with the user:
  - `opsx-explore <topic>`
- Generate the `openspec` change artifacts in one shot:
  - `opsx-new <change-name>`
  - `opsx-ff <change-name>`
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - `./build.ps1 -Tasks test`
  - `opsx-verify <change-name>`
- Locally commit the working `openspec` artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`; e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - `nhc-opsx-commit`
- Locally archive the `openspec` completed change artifacts:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
  - `git add ./openspec/`
  - `nhc-opsx-commit`
### Iterative Exploration to Implementation
Can be used for complex changes or changes with unclear requirements.
- Interactively research and investigate a change with the user:
  - `opsx-explore <topic>`
- Generate the `openspec` change artifacts in one shot:
  - `opsx-new <change-name>`
  - `opsx-continue <change-name>` iteratively and interactively with the user until all artifacts have been accepted.
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - `./build.ps1 -Tasks test`
  - `opsx-verify <change-name>`
- Locally commit the working `openspec` artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`; e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - `nhc-opsx-commit`
- Locally archive the `openspec` completed change artifacts:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
  - `git add ./openspec/`
  - `nhc-opsx-commit`