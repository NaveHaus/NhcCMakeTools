---
name: conventional-commits
description: Use the Conventional Commits v1.0.0 standard to generate git messages. Use to generate semantically consistent commit messages that support automated changelog generation and semantic versioning.
license: MIT
---

# Conventional Commits Skill

Write commit messages following Conventional Commits v1.0.0.

## Requirements
- Follow all steps exactly as presented
- Do not add, remove or modify steps

## Prerequisites
1. Determine if there are changes to be committed:
  - Evaluate the output of `git diff --name-only --staged`:
  - If no output was returned:
    - Announce that no staged changes were found and suggest that user stage the changes to be commited
    - STOP

  - If an error was returned:
    - If the output contains "error: unknown option 'staged'":
      - Announce that the current working directory is probably not a git repository.
      - Give the user the option of manually specifying the repository directory:
        - If the user chooses not to provide a directory:
          - STOP
        - Otherwise: proceed with the next step.
    - Otherwise:
      - Present the user with a summary of the error.
      - Suggest 1-2 remediation strategies if you understand the error and how to fix it. Do NOT fabricate or guess at remediation strategies.
      - Suggest re-running the command after the error has been corrected.
      - STOP

## Required Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

## Rules

1. Prefix with type, optional scope in parentheses, `!` for breaking changes, colon, then space
2. Description immediately follows the colon and space
3. Body separated by one blank line, free-form
4. Footers one blank line after body, token-separator-value format
5. Indicate a breaking change by including `!` before `:`

## Commit Types

| Type | Usage |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes only |
| `style` | Code formatting, whitespace (not for feature or fix changes) |
| `refactor` | Code restructuring (not for feature, fix, or style changes) |
| `perf` | Performance improvement changes |
| `test` | Adding/fixing tests |
| `build` | Build system, build configuration, dependencies |
| `ci` | CI/CD configuration |
| `chore` | Maintenance, tooling tasks |
| `revert` | Reverting previous commit |


## Determining the Message Type

- Does it add new functionality? `feat`
- Does it fix broken functionality? `fix`
- Does it only affect documentation? `docs`
- Does it only change code style/formatting? `style`
- Does it restructure code without changing behavior? `refactor`
- Does it improve performance? `perf`
- Does it add/modify tests? `test`
- Does it change build system or dependencies? `build`
- Does it change CI/CD configuration? `ci`
- Is it maintenance or tooling? `chore`
- Does it undo a previous commit? `revert`

## Best Practices

### Description
- Use fewer than 50 characters if the result is sufficiently clear, up to 72 otherwise
- Use an imperative mood ("add" not "added")
- Do not capitalize the first letter
- Do not add a period at the end
- Try to clearly and succinctly summarize WHAT was done in the description so that the scope can normally be omitted

### Scope
- The scope should generally be a defined capability or section of the codebase, not to a single file or function
- Include a scope only if:
  - It cannot be clearly inferred from the description what part of the codebase changed; or
  - The user provides an explicit scope
- Never include a scope if the user explicitly asks for it to be omitted
- When the scope is included, use clear, consistent, and brief names; e.g. `feat(auth)`, `fix(api)`, `docs(user)`, `docs(dev)`

### Body
- Include only when a change requires more explanation than can be provided in the description
- If a body is included:
  - Succinctly describe what problem the change solves
  - Explain WHY the change was made, not HOW or WHAT was changed
  - Wrap at 72 characters per line

### Footers
- Do not include any footers unless explicitly requested by the user

### General
- Prefer a single-line (i.e. description only) commit message when the number and/or scope of changes is small; e.g.:
  - No more than 20 single-line changes across 5 or fewer files
  - The addition of 2 or fewer files with a well-defined purpose that can be described in a single line
- If unsure about the commit message format or content, or if the content to be summarized is ambiguous or unclear, ask the user for clarification

## Steps
1. Determine what should be summarized in the commit message:
  - If the content to be summarized has been specified prior to invoking this skill:
    - Proceed to Step 2.
  - Otherwise: use the output of `git diff --staged` as the content to summarize

2. Prepare the commit message:
  - Run `git log --oneline -5` to review and incorporate the style of recent commit messages
  - Determine the message type from the content to be summarized by following [Determining the Message Type](#determining-the-message-type)
  - Generate the commit message from the content to be summarized by following [Best Practices](#best-practices)

3. Complete the commit:
   - Show the commit message to the user
   - Use a suitable "**Ask**"-like tool if available to present the user with the following options:
     - Use the generated commit message
     - Enter a custom commit message
   - Present the full git commit command to the user based on the selection: `git commit -m <message>`
   - Ask the user for confirmation that the command is acceptable:
    - If the user confirms that the command is acceptable:
      - Complete the commit by running `git commit -m <message>`
   - Otherwise:
     - Announce that the commit has been cancelled
     - STOP

## References
- [Conventional Commits specification v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/#specification)