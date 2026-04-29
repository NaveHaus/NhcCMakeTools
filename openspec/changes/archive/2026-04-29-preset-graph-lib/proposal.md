## Why

The NhcCMakeTools project requires a robust mechanism for visualizing and interacting with CMake preset collections. CMake presets inherently form complex dependency structures via `include` paths (file inclusions) and `inherits` lists (preset inheritance). Furthermore, the validation of these structures depends on macro expansion and the evaluation of complex `condition` expressions. Without a dedicated library to parse, model, and dynamically re-evaluate these structures as Directed Acyclic Graphs (DAGs), building an interactive visualization or configuration UI is exceedingly difficult and prone to errors. The primary consumer is an interactive UI, so the library also needs stable topology and actionable diagnostics even while some macro inputs remain unknown.

## What Changes

- Introduce a new C++ library (`NhcPresetGraph`) for modeling CMake presets as graphs.
- Establish core graph theory abstractions (Nodes, Edges, Graphs) tailored for preset data.
- Implement independent DAG models for `include` relationships and `inherits` relationships.
- Implement a macro context system to dynamically evaluate macro expressions.
- Implement an Abstract Syntax Tree (AST) to evaluate CMake preset `condition` objects against a macro context.
- Implement a composite `PresetsGraph` manager to orchestrate parsing, context application, and state computation across the two DAGs.
- Support both `CMakePresets.json` and `CMakeUserPresets.json` roots, including the v1 rule that `CMakeUserPresets.json` auto-includes a sibling `CMakePresets.json` when present.
- Validate workflow preset step compatibility against the initial configure preset while keeping workflow presets model-only for inheritance.
- Define the v1 compatibility boundary: structural preset rules track the CMake specification, but macro expansion remains diagnostic-friendly rather than strict CMake execution emulation.
- Standardize JSON parsing on `nlohmann/json` and define a file loader abstraction for loading preset files as strings.

## Capabilities

### New Capabilities
- `preset-graph-core`: Core graph theory structures (nodes, edges, DAG base).
- `preset-macro-context`: System for expanding macros in strings.
- `preset-condition-ast`: Parsing and evaluation of CMake preset `condition` logic.
- `preset-file-loader`: Abstraction for loading preset files by path as strings.
- `preset-include-graph`: Graph model specifically for file `include` relationships.
- `preset-inheritance-graph`: Graph model specifically for preset `inherits` relationships.
- `preset-model`: Typed preset model that retains raw preset JSON and per-preset resolved field state for inheritance and macro re-evaluation.
- `preset-graph-manager`: The composite manager that drives graph state resolution.

These capabilities are implemented as modules within the `NhcPresetGraph` library.

For v1, the library contract follows CMake's structural rules for includes, inheritance, and workflow-step compatibility while intentionally preserving unresolved macro references when that yields better diagnostics for interactive tooling.

### Modified Capabilities

(None. This is a wholly new set of capabilities.)

## Impact

- **Code**: Adds a new C++ static library to the project workspace.
- **APIs**: Provides a new API surface for interacting with CMake presets programmatically.
- **Dependencies**: Adds a dependency on `nlohmann/json` for parsing preset JSON.
