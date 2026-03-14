---
name: tdd
description: Test-driven development with red-green-refactor loop. Use to methodically implement new features, improve existing features, fix bugs, etc.
metadata:
  references:
    - https://github.com/mattpocock/skills/blob/main/tdd/SKILL.md
---

# Test-Driven Development (TDD)

## Philosophy

**Main goal**: Produce correct, test-covered behavior with minimal speculative design.

**Core principles**:
- Verify behavior through public interfaces, not implementation details.
- Code can change, tests should not change.
- Only implement code required to satisfy a failing test.

**Good tests**:
- Exercise real code paths through public APIs.
- Describe _what_ the system does, not _how_ it does it.
- Reads like a specification that says exactly what capability exists.

**Bad tests**:
- Coupled to implementation.
- Mock internal collaborators, test private methods, or verify through external means (like querying a database directly instead of using the interface).
- Warning signs:
  - A test breaks after a refactor, but behavior hasn't changed.
  - Renaming an internal function causes tests to fail (tests were testing implementation, not behavior).

## Approach

**Wrong Appropach (Horizontal)**: treating RED as "write all tests" and GREEN as "write all code."
- Tests _imagined_ behavior, not _actual_ behavior.
- Results in testing the _shape_ of things (data structures, function signatures) rather than user-facing behavior.
- Tests become insensitive to actual changes.
- Tests pass when behavior breaks, fail when behavior is fine.
- Results in committing to test structure before understanding the implementation.

```
WRONG (horizontal):
  RED:   test1, test2, test3, test4, test5
  GREEN: impl1, impl2, impl3, impl4, impl5
```

**Correct Approach (Vertical)**: One test → one implementation → repeat.
- Each test incorporates lessons learned from the previous iteration.

```
RIGHT (vertical):
  RED→GREEN: test1→impl1
  RED→GREEN: test2→impl2
  RED→GREEN: test3→impl3
  ...
```

## Workflow

1. IF the user does not provide a specification of what to test, ask.
2. Choose ONE behavior defined by the user-provided specification, then write ONE test:
   ```
   RED:   Write test → test fails
   GREEN: Write minimal code to pass → test passes
   ```
   - **Decision Guidelines**:
     - Prefer:
 
       * The simplest success case first;
       * THEN boundary cases;
       * THEN error cases.
     - Avoid behaviors that:
 
       * Require multiple concerns simultaneously.
       * Depend on speculative abstractions.
       * Test implementation rather than outcomes.
     - IF multiple choices remain, pick the one that:
 
       * Reduces uncertainty; and/or
       * Avoids unnecessary design commitments.
3. Iterate (REFACTOR):
   - Once ALL tests pass for the user-provided specification, consider refactoring ONLY **within the scope of the specification**:
     - **Duplication** → Extract function/class
     - **Long methods** → Break into private helpers (keep tests on public interface)
     - **Shallow modules** → Combine or deepen
     - **Feature envy** → Move logic to where data lives
     - **Primitive obsession** → Introduce value objects
     - **Existing code** the new code reveals as problematic

**Rules**:
- DO write ONE test at a time.
- DO write ONLY enough code to pass the current test.
- DO test ONLY observable behavior.
- DO assert ONLY on **observable behavior**.
- DO ensure each failure occurs for the **correct reason**.
- DO NOT anticipate future tests.
- DO NOT implement code outside the bounds of the user-provided specification.

## Definition of Done
- All behaviors defined by the user-provided specification have tests AND
- All tests pass AND
- Code is clean and satistifies the requirements of the user-provided specification.