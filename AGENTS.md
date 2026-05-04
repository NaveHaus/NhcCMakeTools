# Context, Rules, and Guidelines for AI Agents
**Table of Contents**
- [Overview](#overview)
- [Tech stack](#tech-stack)
- [Safety & Operational Rules (MANDATORY)](#safety--operational-rules-mandatory)
- [Process](#process)
  - [Prerequisites (MANDATORY)](#prerequisites-mandatory)
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

## Overview
The NhcCMakeTools project hosts tools that focus on improving quality-of-life for developers working with the [CMake](https://cmake.org) cross-platform build system

## Tech stack
- c++23
- clang-format
- CMake 3.31+
- Qt 6.9
- Catch2 v3.11+
- Doxygen 1.16+
- GitHub Flavored Markdown
- OpenSpec

## Safety & Operational Rules (MANDATORY)
To maintain repository integrity, agents MUST follow these rules:
- **No Secrets**: NEVER commit secrets, API keys, or credentials
- **Git Safety**:
  - DO NOT modify git configuration
  - DO NOT use `--no-verify` or bypass hooks unless explicitly requested
  - DO NOT force push to protected branches, e.g. `master` or `main`
  - DO NOT automatically fix `git` errors---ALWAYS ask the user for confirmation first
  - DO warn the user if attempting to commit to `master` or `main`
  - DO warn the user if `git` returns an error or reports that the remote branch is missing
  - DO offer to resolve `git` errors, presenting the user with 1-4 options for doing so
- **Path Handling**: Always use absolute paths when interacting with file system tools
- **Build Consistency**: DO NOT directly modify or remove any files or directories listed in `.gitignore`
- **Verification**: Always run build and test commands after modifications

## Process

### Prerequisites (MANDATORY)
- Warn the user if the `openspec` directory is missing or inaccessible in the current workspace
- Warn the user if the `tdd` skill is unavailable
- Warn the user if the `nhc-openspec-commit` skill is unavailable
- Warn the user if the `nhc-conventional-commit` skill is unavailable

### Requirements (MANDATORY)
- You MUST use an OpenSpec workflow to plan new features and modifications of existing features
- You MUST use the `nhc-openspec-commit` skill to generate a `git` commit message and commit changes made within an OpenSpec workflow
- You MUST use the `nhc-conventional-commit` skill to generate a `git` commit message for changes made outside an OpenSpec workflow
- You MUST use the test-driven development (TDD) `tdd` skill:
  - To generate testing tasks in the OpenSpec `tasks.md` artifact
  - To test new code
  - To test changes to existing code
  - To update ALL tests affected by adding or changing code
- You MAY skip testing for non-code changes, e.g. documentation changes
- You MUST store ALL tests under `tests/<category>`, where `<category>` MUST be the name of the component (e.g. library, tool, etc.) containing the feature/class under test

### Major Features
- Major features are comprised of two or more distinct OpenSpec changes, e.g. an MVP for a new front-end user interface, the implementation of a new back-end service, or resolving multiple related code issues
- You MUST use `openspec-explore` to research and develop a plan for implementing the changes comprising a major feature
- You MUST use the `docs/plans/<major-feature-name>` directory as a workspace during `openspec-explore`:
  - DO NOT store OpenSpec artifacts under `docs/plans/<major-feature-name>`
  - You MUST ignore ALL files in `docs/plans/archive` unless the user explicitly requests to review them
  - Before creating planning documents, you MUST **ask the user** to provide the name of the major feature if the user has not already provided one
  - You MUST review all relevant planning documents for context while developing `<major-feature-name>`
  - You MUST offer to record decisions, constraints, milestones, dependencies, or any other important information relevant to planning the major feature
  - You MUST offer to create or update a current list of changes as they become clear
  - You MUST include a dependency diagram when recording the current list of changes, if the dependencies are clear; prefer a mermaid diagram for ease-of-use
  - You MAY store high-level planning files directly under `docs/plans`, e.g. project-level guidance or decisions rather than feature-level information
  - You MAY offer to create the OpenSpec artifacts for a concrete, implementable change, but they MUST be stored in the correct directory under `openspec`
  - Prefer `openspec-ff-change` over `openspec-new`/`openspec-continue` when creating change artifacts for a major feature

**Most common OpenSpec workflow:**
2. `openspec-ff-change`: Create all change artifacts in one go
3. `nhc-openspec-refine`: Make artifacts implementation-ready (this step may be repeated until no new issues are found)
4. `openspec-apply-change`: Implement the tasks in `tasks.md` following the TDD RED/GREEN/REFACTOR process
5. `cmake --workflow --preset=clangd-ninja-vcpkg-release-test`: Verify all tests before completing the change
6. `nhc-openspec-verify`: Validate implementation against specs (this step may be repeated until no new issues are found)
7. `openspec-sync-specs`: Make OpenSpec spec files permanent
8. `openspec-archive-change`: Archive the OpenSpec change
9. `nhc-openspec-commit`: Generate a standard `git` commit message for the changes made by the OpenSpec workflow

## Code Style
### Requirements (MANDATORY)
- DO use `nhc_*` CMake functions found in `cmake/NhcTargetFunctions.cmake` to configure projects, tests, and executables
- DO use camel-case for class names, e.g. `class MyNewClass`
- DO use `<Class>.h`/`<Class>.cpp` when implementing new classes, where `<Class>` is the exact, camel-case name of the class to be implemented, e.g. `MyNewClass.h`/`MyNewClass.cpp`
- DO use `m_<Member>` for class member variables, where `<Member>` is a camel-case variable name, e.g. `m_EnableDebug`
- DO use `Get<Member>()`/`Set<Member>()` for setter-getter class methods that access a member variable, where `<Member>` is the `m_<Member>` member variable
- DO use `<Member><Property>()` for pure observers that cannot be directly modified by a caller, e.g. `ItemCount()`
- Do use `<Action><Property>(<args>)` for updating properties that can be counted, e.g. `AddItem(<args>)`/`RemoveItem(<args>)`/`SetItem(<args>)`
- DO run `clang-format -i <file>.cpp` after modifying C++ source code to ensure the code is formatted properly
- DO use C++-style Doxygen comments for developer-facing class documentation, e.g.:
  ```cpp
  /// Base class for all FibbetyGibbet managers
  ///
  /// ... More detailed explanation of the class behavior and general usage requirements ..
  class AbstractFibbetyGibbetManager {
    protected:
      /// Default constructor
      ///
      /// Has no side-effects
      AbstractFibbetyGibbetManager();

      /// Virtual destructor
      ///
      /// Has no side-effects
      virtual ~AbstractFibbetyGibbetManager();

    public:
      /// Retrieves the number of fibbeties
      ///
      /// Must be overridden by derived classes
      /// @returns The number of fibbeties managed by this instance
      virtual unsigned FibbetyCount() = 0;

      /// Adds a new gibbety to this instance
      ///
      /// Must be overridden by derived classes
      virtual void AddGibbety() = 0;

      ... rest of class implementation ..
  };
  ```

## Testing

### Requirements (MANDATORY)
- Test files MUST follow the naming convention: `tests/<category>/<feature>Tests.cpp`, e.g. `tests/MyLibName/MyClassTests.cpp` for a library class test, or `tests/MyToolName/MyToolSpecificClassTests.cpp` for an executable tool
  - `<feature>` will normally be the name of a C++ class under test, e.g. `MyClassTests.cpp`
  - The test must be registered with CMake by modifying `tests/<category>/CMakeLists.txt` to call the `nhc_add_test_executable()` CMake function, defined in `cmake/NhcTargetFunctions.cmake`, e.g.:
    ```cmake
    nhc_add_test_executable(<feature>Tests SOURCES <feature>Tess.cpp USES NhcTestLib)
    ```
    - Note: `USES NhcTestLib` ensures Catch2 is configured properly for the test
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
  - Note: If you think a BDD test cannot be used or does not apply, iterate with the user to come up with a workable solution

### Edit-Build-Test Commands (MANDATORY)

- **Listing Available Presets**
  - `cmake --list-presets=configure`: List all CMake presets available to generate build files for specific compilers and operating environments
  - `cmake --list-presets=build`: List all CMake presets available to execute a configured build for specific compilers and operating environments
  - `cmake --list-presets=test`: List all CMake presets available to test a configured and executed build for specific compilers and operating environments
  - `cmake --list-presets=workflow`: List all CMake presets available to execute an end-to-end configure/build/test cycle for specific compilers and operating environments

- **Configuring a Build**
  - `cmake --preset=<preset>`: Configure the `<preset>` build. Example to configure statically linked Debug and Release builds with clang:
    ```powershell
    cmake --preset=clang-clangd-ninja-vcpkg-mt-s
    ```
  - **Result**: Creates the `.build/clang-clangd-ninja-vcpkg-mt-s` binary directory with the configured build

- **Executing a Build**
  - `cmake --build --preset=<preset>`: Build the `<preset>` build. The build MUST be configured first. Example to execute a statically-linked clang Release build:
    ```powershell
    cmake --build --preset=clang-clangd-ninja-vcpkg-mt-s
    ```
  - **Result**: Executes a Release build under `.build/clang-clangd-ninja-vcpkg-mt-s`, generating all library and executable artifacts

- **Executing All Tests**
  - `ctest --preset=<preset>`: Test the `<preset>` build. The build MUST be configured AND built first. Example to test statically-linked clang Release build:
    ```powershell
    ctest --preset=clang-clangd-ninja-vcpkg-mt-s-release
    ```
  - **Result**: Executes all tests for a Release build under `.build/clang-clangd-ninja-vcpkg-mt-s`

- **One-Shot Configuring, Executing, and Testing a Build**
  - `cmake --workflow --preset=<preset>-test`: Configure, execute, and test the `<preset>` build. Example to configure, build, and test staticall-linked clang Release build:
    ```powershell
    cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test
    ```
  - **Result**: Executes all tests for a Release build under `.build/clang-clangd-ninja-vcpkg-mt-s`

- **Building and Testing in a TDD Cycle**
  - After modifying a `CMakeLists.txt` file, execute `cmake --preset=<preset>` to reconfigure the `<preset>` build
  - After making changes to create or update `<feature>Tests.cpp`:
    - Execute `cmake --build --preset=<preset> --target <feature>Tests` to build the modified test
    - Execute `ctest --preset=<preset> --test-regex <feature>Tests` to execute the modified test

- **Verification Testing after a TDD Cycle**
  - After completing a TDD cycle to implement a new feature or modify an existing feature, verify correctness by executing a workflow preset appropriate for the current environment, e.g. Linux with clang on the path:
    ```powershell
    cmake --workflow --preset=clang-clangd-ninja-vcpkg-mt-s-release-test
    ```

- **Ensuring a clangd-based C++ LSP Sees Changes**
  - If a C++ LSP seems out-of-date, force CMake to reconfigure the project:
    ```powershell
    cmake --preset=clang-clangd-ninja-vcpkg-mt-s
    ```

## Example OpenSpec Workflows

### One-Shot Implementation
Can be used for simple changes that require no investigation or decision making prior to implementation:
- Generate and review the OpenSpec change artifacts in one shot:
  - `openspec-ff-change <change-name>`
  - `nhc-openspec-refine <change-name>`
- Implement and verify the change:
  - `openspec-apply-change <change-name>`
- Implement and verify the change:
  - `openspec-apply-change <change-name>`
  - See [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory) for the required commands to use to implement the change
  - `nhc-openspec-verify <change-name>`
- Archive the completed OpenSpec change:
  - `openspec-sync-specs <change-name>`
  - `openspec-archive-change <change-name>`
- Locally commit the working OpenSpec artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`, e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/src ${pwd}/tests
    ```
  - Use the `nhc-openspec-commit` skill to complete the `git` commit

### One-Shot Exploration to Implementation
Can be used for straightforward changes that require some upfront investigation and/or decision making prior to implementing
- Interactively research and investigate a change with the user:
  - `openspec-explore <topic>`
- Generate and review the OpenSpec change artifacts in one shot:
  - `openspec-ff-change <change-name>`
  - `nhc-openspec-refine <change-name>`
- Implement and verify the change:
  - `openspec-apply-change <change-name>`
  - See [Edit-Build-Test Commands (MANDATORY)](#edit-build-test-commands-mandatory) for the required commands to use to implement the change
  - `nhc-openspec-verify <change-name>`
- Archive the completed OpenSpec change:
  - `openspec-sync-specs <change-name>`
  - `openspec-archive-change <change-name>`
- Locally commit the working OpenSpec artifacts and associated project changes:
  - `git add ./openspec/ <changed-files-and-or-directories>`, e.g.:
    ```bash
    git add ${pwd}/openspec ${pwd}/src ${pwd}/tests
    ```
  - Use the `nhc-openspec-commit` skill to complete the `git` commit