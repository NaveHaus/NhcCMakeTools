## Why

The NhcCMakeTools project requires a robust mechanism for visualizing and interacting with CMake preset collections. CMake presets inherently form complex dependency structures via `include` paths (file inclusions) and `inherits` lists (preset inheritance). Furthermore, the validation of these structures depends on macro expansion and the evaluation of complex `condition` expressions. Without a dedicated library to parse, model, and dynamically re-evaluate these structures as Directed Acyclic Graphs (DAGs), building an interactive visualization or configuration UI is exceedingly difficult and prone to errors.

## What Changes

- Introduce a new C++ library (`nhc-preset-graph`) for modeling CMake presets as graphs.
- Establish core graph theory abstractions (Nodes, Edges, Graphs) tailored for preset data.
- Implement independent DAG models for `include` relationships and `inherits` relationships.
- Implement a macro context system to dynamically evaluate macro expressions.
- Implement an Abstract Syntax Tree (AST) to evaluate CMake preset `condition` objects against a macro context.
- Implement a composite `PresetsGraph` manager to orchestrate parsing, context application, and state computation across the two DAGs.

## Capabilities

### New Capabilities
- `preset-graph-core`: Core graph theory structures (nodes, edges, DAG base).
- `preset-macro-context`: System for expanding macros in strings.
- `preset-condition-ast`: Parsing and evaluation of CMake preset `condition` logic.
- `preset-include-graph`: Graph model specifically for file `include` relationships.
- `preset-inheritance-graph`: Graph model specifically for preset `inherits` relationships.
- `preset-graph-manager`: The composite manager that drives graph state resolution.

### Modified Capabilities

(None. This is a wholly new set of capabilities.)

## Impact

- **Code**: Adds a new C++ static library to the project workspace.
- **APIs**: Provides a new API surface for interacting with CMake presets programmatically.
- **Dependencies**: May require dependencies on a JSON parser (e.g., nlohmann/json or similar) to read the raw preset files.
