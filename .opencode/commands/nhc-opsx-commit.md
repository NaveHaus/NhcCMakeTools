---
name: nhc-opsx-commit
description: Commit staged changes
agent: openspec
---

Commit staged changes that optionally correspond to an Openspec change.

**Input**: Optionally specify a change name (e.g., `/openspec-commit add-auth`).

**Steps**

1. **If a change name is NOT provided**
   - Give the user the option of selecting an existing openspec change:
     - If the user opts to select an existing change:
       - Run `openspec list --json` to get available changes.
       - Use any suitable "**Ask**"-like tool if available to let the user select.
       - If the user selects a change, continue with the "If a change name is provided:" instructions.
     - Otherwise: continue with "Generate the commit message:" instructions.

2. **If a change name is provided**
   - Announce "Generating commit for change: <name>" and how to override (e.g., `/opensec-commit <other>`).
   - Parse the JSON output of `openspec status --change "<name>" --json`:
     - If a JSON object was returned, make note of:
       - `schemaName`: The workflow being used (e.g., "spec-driven")
       - Which artifact contains the tasks (typically "tasks" for spec-driven, check status for others)
     - Otherwise:
       - Present the exact error message to the user.
       - STOP
   - Parse the output of `openspec instructions apply --change "<name>" --json`:
     - Handle the state:
       - If `state: "blocked"` (missing artifacts):
         - Show message, suggest using `/opsx-continue`.
         - STOP
     - Determine the path to the artifact containing tasks:
       - If the path to the tasks artifact can not be reliably determined from the output, ask the user to provide the path.
       - If the user does not provide the path:
         - STOP
     - **Content to summarize**: ONLY use the contents of the tasks artifact to create the commit message

3. **Generate the commit message**
   - Invoke the conventional-commits skill to complete the commit.