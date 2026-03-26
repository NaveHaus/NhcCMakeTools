## Context

The NhcCMakeTools project needs to visualize and interactively manipulate CMake preset collections. CMake presets are defined across multiple JSON files using `include` statements, and individual presets inherit from each other using `inherits` lists. Both the file inclusions and preset definitions can utilize CMake macros (e.g., `${presetName}`, `$env{FOO}`) which dictates their validity and structure. Furthermore, presets can have complex `condition` objects that determine if they are enabled or disabled, and these conditions can also contain macros. 

To build an interactive UI, we need a data structure that can parse the raw, unexpanded presets and dynamically re-evaluate their logical structure as the user changes macro values.

## Goals / Non-Goals

**Goals:**
- Provide a robust C++ data model for CMake preset inheritance and file inclusion.
- Accurately represent the constraints of the CMake specification (e.g., uniqueness of names, specific macro rules).
- Allow dynamic re-evaluation of the graph structure based on a mutable "macro context".
- Clearly distinguish between the structural state of the graph (topology) and the data within the nodes.
- Expose clear APIs to query the state of nodes (e.g., Enabled vs Disabled based on conditions) for UI rendering.

**Non-Goals:**
- Writing or serializing JSON files back to disk (this is a read/evaluate library).
- Execution or invocation of CMake itself.
- Validating the actual build configurations (we only validate the structural rules of the preset JSON spec).

## Decisions

### 1. Dual Directed Acyclic Graphs (DAGs)
**Decision**: Implement two separate but related DAGs: `PresetIncludeGraph` (for file inclusions) and `PresetInheritanceGraph` (for preset inheritance).
**Rationale**: File inclusions and preset inheritance are independent concepts in CMake. A file can include another file without sharing any preset inheritance. Managing them as separate graphs simplifies cycle detection and node management.
**Alternatives Considered**: A single heterogeneous graph containing both File Nodes and Preset Nodes. Rejected because the edges have fundamentally different meanings, making graph traversals and cycle detection overly complex.

### 2. Separation of Topology from Payload
**Decision**: Base graph data structures will use standard graph theory representations (Nodes identified by `unsigned int` IDs, Edges managed by the graph as ID pairs). The actual file/preset data will be stored as payloads associated with these IDs.
**Rationale**: CMake allows cyclic dependencies in theory (though they are invalid), and users might type them in. By managing edges purely as integer pairs, we can implement standard cycle detection and topological sorting algorithms without polluting the business logic objects. It also prevents object pointer invalidation issues during graph recomputations.

### 3. "Structural" vs "Cosmetic" State
**Decision**: The "State" of the graphs (`Empty`, `Unresolved`, `Resolved`) will reflect ONLY the structural topology of the graphs, not the resolution of every single macro in every variable.
**Rationale**: The UI needs to know if it has the complete picture. If an `include` path cannot be expanded, the graph is structurally `Unresolved` because we might be missing nodes. However, if a preset's environment variable macro isn't expanded, the structure is fine, it's just a cosmetic evaluation issue for that specific node. 
- `PresetIncludeGraph` is Resolved when all `include` paths are fully expanded.
- `PresetInheritanceGraph` is Resolved when all `inherits` names point to known presets AND all `condition` objects can be evaluated.

### 4. Condition Abstract Syntax Tree (AST)
**Decision**: CMake preset `condition` objects will be parsed into an AST where each node represents a condition type (e.g., `Equals`, `AllOf`, `InList`). The AST nodes will provide an `Evaluate(MacroContext)` method.
**Rationale**: Conditions can be deeply nested and contain macros. An AST allows recursive evaluation against the current user context.
**Alternatives Considered**: Flattening conditions into a list. Rejected because CMake conditions inherently support nested boolean logic (`anyOf`, `allOf`, `not`).

### 5. Retaining "Disabled" Nodes
**Decision**: Presets that evaluate to `false` based on their `condition` are kept in the graph but marked with a `Disabled` status.
**Rationale**: In an interactive UI, a user might change a macro that causes a condition to switch from `false` to `true`. If the node was pruned from the graph, it would abruptly pop into existence. By keeping it but marking it `Disabled`, the UI can choose to render it as grayed-out, providing better feedback to the user.

### 6. Graceful Partial Macro Expansion
**Decision**: When expanding a string containing macros, missing macros will result in a partially expanded string (or an indicator of partial expansion) rather than throwing an error or failing completely.
**Rationale**: Because graph resolution is an iterative, interactive process driven by user input, it is completely normal and expected for macros to be temporarily missing. Treating partial expansion as a failure would break the iterative resolution strategy. Instead, a partially expanded string naturally causes the dependent node to remain in an `Unresolved` state until the user provides the necessary macro.

**Deviation from CMake**: CMake specifies that missing `$env{NAME}` / `$penv{NAME}` references evaluate to an empty string. This library keeps missing references unresolved so the UI can surface them and the user can explicitly decide whether an empty string is intended.

### 7. File Loader Abstraction + nlohmann/json
**Decision**: The `PresetsGraph` manager will accept an injected file loader abstraction whose responsibility is to read a preset file path and return its contents as a string. The manager will parse loaded content using `nlohmann::json::parse(...)`.
**Rationale**: The graph manager necessarily follows `include` references to discover additional preset files. Injecting file loading keeps the manager testable and keeps file system policy (sandboxing, virtual files, in-memory fixtures) outside the core graph logic. Standardizing on `nlohmann/json` provides a widely used, vcpkg-friendly parser that can parse from `std::string`.
**Alternatives Considered**:
- Passing pre-parsed JSON objects into the manager. Rejected because the manager must iteratively load newly discovered include files.
- Implementing full IO and path policy inside the manager. Rejected because it complicates testing and makes sandboxed/virtual file systems difficult.

**Path Resolution Policy (v1)**:
- When an include path is relative, the manager resolves it relative to the directory of the including preset file.
- The include graph SHOULD store normalized absolute paths for file nodes to ensure stable identity across reloads.

**Environment Policy (v1)**:
- `$env{}` and `$penv{}` do not read the actual process environment. The parent/process environment is provided explicitly via `MacroContext`.
- The graph manager MAY inject preset-associated values into `MacroContext` during traversal (e.g., `${presetName}` for the active preset).

### 8. Non-Fatal Resolution Diagnostics
**Decision**: Missing include files and invalid JSON are non-fatal resolution outcomes. They do not cause the graph manager to throw or abort the full apply-context operation. Instead, the corresponding file node is retained in the Include Graph and marked `Unresolved` with an `UnresolvedReason`.
**Rationale**: In interactive use, users frequently edit presets and file layouts. Retaining missing/invalid nodes gives the UI a stable structure to render with actionable diagnostics, and allows resolution to complete for other independent branches of the include tree.
**UnresolvedReason (initial set)**:
- `FileDoesNotExist`: The file loader could not find the file.
- `InvalidJson`: The file content could not be parsed as JSON.
- `MissingMacro`: An include path could not be fully expanded due to missing macro or environment values.
- `UnsupportedMacro`: A string used a macro that is disallowed by the preset specification.
- `EnvironmentCycle`: Environment values contain a reference cycle.
- `IncludeCycle`: Preset files contain an include cycle.
- `InheritanceCycle`: Presets contain an inheritance cycle.

## Risks / Trade-offs

- **Risk**: Infinite loops during dynamic discovery. If a user sets a macro that causes an `include` path to point to a file we've already loaded, which points back to the first file, we could get stuck.
  - *Mitigation*: The `PresetsGraph` manager MUST implement strict cycle detection during the include resolution loop, halting discovery and marking the graph as invalid/unresolved if a cycle is detected.
- **Risk**: Performance of frequent re-evaluation. Rebuilding the graphs on every keystroke in a UI could be slow if the preset tree is massive.
  - *Mitigation*: The dual-graph approach allows us to short-circuit. If a macro changes that only affects a `condition`, we only need to re-evaluate the `PresetInheritanceGraph`, saving the cost of re-parsing file paths.
