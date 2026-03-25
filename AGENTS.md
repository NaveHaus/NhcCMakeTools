# Context, Rules, and Guidelines for AI Agents
The NhcCMakeTools project hosts tools that focus on improving quality-of-life for developers working with the [CMake](https://cmake.org) cross-platform build system.

## Sections
- [Tech stack](#tech-stack)
- [Safety & Operational Rules (MANDATORY)](#safety--operational-rules-mandatory)
- [Terminology](#terminology)
- [Process](#process)
  - [Requirements (MANDATORY)](#requirements-mandatory)
- [Code Style](#code-style)
  - [Requirements (MANDATORY)](#requirements-mandatory-1)
- [Testing](#testing)
  - [Requirements (MANDATORY)](#requirements-mandatory-2)
  - [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory)
- [Example OpenSpec Workflows](#example-openspec-workflows)
  - [One-Shot Implementation](#one-shot-implementation)
  - [One-Shot Exploration to Implementation](#one-shot-exploration-to-implementation)
  - [Iterative Exploration to Implementation](#iterative-exploration-to-implementation)

## Tech stack
- c++23
- clang-format
- CMake 3.31+
- Qt 6.9
- Catch2 v3.11+
- Doxygen 1.16+
- GitHub Flavored Markdown

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
- **Build Consistency**: DO NOT directly modify or remove any files under the `.build` directory.
- **Verification**: Always run build and test commands after modifications.

## Terminology
- **OpenSpec**: An artifact-driven workflow for managing software changes (features, fixes, etc.) through structured specifications and tasks.
- **openspec-***: Agent skills for interacting with an OpenSpec change workflow.
- **convetional-commits**: An agent skill for generating a `git` commit message following the Conventional Commits v1.0 specification.
- **nhc-openspec-commit**: A skill that uses the `conventional-commits` skill to generate compliant `git` commit messages summarizing the changes made by an OpenSpec workflow.

## Process
### Requirements (MANDATORY)
- An OpenSpec workflow MUST be used to plan new features and modifications of existing features (e.g. bug fixes). Warn the user if the `openspec` directory is missing or inaccessible.
- A test-driven development (TDD) RED/GREEN/REFACTORING workflow is required when implementing ALL code additions and modifications. Warn the user if the `tdd` skill is missing or inaccessible.
- The `nhc-opsx-commit` skill MUST be used to generate a `git` commit message and commit changes made WITH an OpenSpec workflow, but only AFTER the OpenSpec change has been archived. Warn the user if the `nhc-opsx-commit` skill is missing or inaccessible.
- The `conventional-commits` skill MUST be used to generate a `git` commit message for changes made OUTSiDE of an OpenSpec workflow. Warn the user if the `conventional-commits` skill is missing or inaccessible.
- NOTE: If unsure about which commit strategy applies, you MUST ask the user to avoid generating spurious or erroneous commits.

Most common sequence of `openspec` operations:
1. `opsx-new <name>`: Initialize the change.
2. `opsx-ff <name>`: Generate/update artifacts.
3. `opsx-apply <name>`: Implement the tasks (following TDD process - see [Testing](#testing)).
4. `./build.ps1 -Tasks test`: Verify all tests before completing the change.
5. `opsx-verify <name>`: Validate implementation against specs (see [Error Handling Guidance](#error-handling-guidance) if this fails).
6. `opsx-sync <name>`: Make OpenSpec specs artifacts permanent.
7. `opsx-archive <name>`: Archive the OpenSpec change.
8. `git add ./openspec/ <paths-to-changed-files-and-directories>`: Stage the archived OpenSpec artifacts and associated changes.
9. `nhc-opsx-commit`: Generate a `conventional-commits` `git` commit message based on the staged changes and complete the commit.

## Code Style
### Requirements (MANDATORY)
- DO use `nhc_*` CMake functions found in `cmake/NhcTargetFunctions.cmake` to configure projects, tests, and executables.
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

## Testing
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
    - Note: `USES NhcTestLib` ensures Catch2 is configured properly for the test.
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
  - Note: If you think a BDD test cannot be used or does not apply, iterate with the user to come up with a workable solution.

### Edit-Build-Test Commands (MANDATORY)

- **Listing Available Presets**
  - `cmake --list-presets=configure`: List all CMake presets available to generate build files for specific compilers and operating environments.
  - `cmake --list-presets=build`: List all CMake presets available to execute a configured build for specific compilers and operating environments.
  - `cmake --list-presets=test`: List all CMake presets available to test a configured and executed build for specific compilers and operating environments.
  - `cmake --list-presets=workflow`: List all CMake presets available to execute an end-to-end configure/build/test cycle for specific compilers and operating environments.

- **Configuring a Build**
  - `cmake --preset=<preset>`: Configure the `<preset>` build. Example to configure a Visual Studio 18 2026 build:
    ```powershell
    cmake --preset=vs18-vcpkg-mt-s
    ```
  - **Result**: Creates the `.build/vs18-vcpkg-mt-s` binary directory with the build configured for Visual Studio 18 2026.

- **Executing a Build**
  - `cmake --build --preset=<preset>`: Build the `<preset>` build. The build MUST be configured first. Example to execute a Visual Studio 18 2026 Release build:
    ```powershell
    cmake --build --preset=vs18-vcpkg-mt-s-release
    ```
  - **Result**: Executes a Release build under `.build/vs18-vcpkg-mt-s`, generating all library and executable artifacts.

- **Executing All Tests**
  - `cmake --test --preset=<preset>`: Test the `<preset>` build. The build MUST be configured AND built first. Example to test a Visual Studio 18 2026 Release build:
    ```powershell
    cmake --test --preset=vs18-vcpkg-mt-s-release-test
    ```
  - **Result**: Executes all tests for a Release build under `.build/vs18-vcpkg-mt-s`.

- **One-Shot Configuring, Executing, and Testing a Build**
  - `cmake --workflow --preset=<preset>-test`: Configure, execute, and test the `<preset>` build. Example to configure, build, and test a Visual Studio 18 2026 build:
    ```powershell
    cmake --workflow --preset=vs18-vcpkg-mt-s-release-test
    ```
  - **Result**: Executes all tests for a Release build under `.build/vs18-vcpkg-mt-s`.

- **Building and Testing in a TDD Cycle**
  - After modifying a `CMakeLists.txt` file, execute `cmake --preset=<preset>` to reconfigure the `<preset>` build.
  - After making changes to create or update `<feature>Tests.cpp`:
    - Execute `cmake --build --preset=<preset> --target <feature>Tests` to build the modified test.
    - Execute `cmake --test --preset=<preset> --test-regex <feature>Tests` to execute the modified test.

- **Verification Testing after a TDD Cycle**
  - After completing a TDD cycle to implement a new feature or modify an existing feature, verify correctness by executing:
    ```powershell
    cmake --workflow --preset=vs18-vcpkg-mt-s-release-test
    ```

## Example OpenSpec Workflows

### One-Shot Implementation
Can be used for simple changes that require no investigation or decision making prior to implementation:
- Generate the OpenSpec change artifacts in one shot:
  - `opsx-new <change-name>`
  - `opsx-ff <change-name>`
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - See [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory) for the required commands to use to implement the change.
  - `opsx-verify <change-name>`
- Archive the completed OpenSpec change:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
- Locally commit the working OpenSpec artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`, e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - Use the `nhc-openspec-commit` skill to complete the `git` commit.

### One-Shot Exploration to Implementation
Can be used for straightforward changes that require some upfront investigation and/or decision making prior to implementing.
- Interactively research and investigate a change with the user:
  - `opsx-explore <topic>`
- Generate the OpenSpec change artifacts in one shot:
  - `opsx-new <change-name>`
  - `opsx-ff <change-name>`
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - See [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory) for the required commands to use to implement the change.
  - `opsx-verify <change-name>`
- Archive the completed OpenSpec change:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
- Locally commit the working OpenSpec artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`, e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - Use the `nhc-openspec-commit` skill to complete the `git` commit.

### Iterative Exploration to Implementation
Can be used for complex changes or changes with unclear requirements.
- Interactively research and investigate a change with the user:
  - `opsx-explore <topic>`
- Iteratively generate the OpenSpec change artifacts:
  - `opsx-new <change-name>`
  - `opsx-continue <change-name>` iteratively and interactively with the user until all artifacts have been accepted.
- Implement and verify the change:
  - `opsx-apply <change-name>`
  - See [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory) for the required commands to use to implement the change.
  - `opsx-verify <change-name>`
- Archive the completed OpenSpec change:
  - `opsx-sync <change-name>`
  - `opsx-archive <change-name>`
- Locally commit the working OpenSpec artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`, e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/CMakeLists.txt ${pwd}/tests/NhcMyLibName/MyClassTests.cpp
    ```
  - Use the `nhc-openspec-commit` skill to complete the `git` commit.