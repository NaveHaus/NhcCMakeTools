## Context

`NhcPresetGraph` already separates responsibility across the include graph, inheritance graph, preset model, and the composite manager. That separation is useful internally, but it leaves common consumers with too much assembly work for basic questions such as:

- Which presets are currently available and why?
- Which file introduced a preset?
- What is the effective resolved state for a preset?
- What relationships exist between files and presets after `ApplyContext(...)`?

The architecture review identifies this as the next consumer-facing gap after the underlying include resolution, inherited-condition handling, manager re-apply behavior, and resolved-state model are corrected. The requested facade must stay thin and read-only so that it benefits from those lower layers instead of re-implementing them.

## Goals / Non-Goals

**Goals:**
- Provide one high-level, read-only query surface for common preset-graph consumers.
- Define facade queries in terms of existing graph/model state produced by the manager after context application.
- Expose effective availability, resolved fields, diagnostics, and common relationships without requiring callers to coordinate multiple subsystems.
- Keep direct access to lower-level graph and model APIs available for advanced callers.

**Non-Goals:**
- Creating another resolution engine, cache, or authoritative data model separate from the manager, include graph, inheritance graph, or preset model.
- Replacing all existing low-level APIs.
- Expanding the facade into mutation, file writing, or context-application responsibilities.
- Hiding every internal distinction between files, presets, inheritance, and workflow semantics.

## Decisions

### 1. The facade is manager-owned and lifecycle-bound to applied context
**Decision**: The query facade will be exposed from the Presets Graph Manager and will describe the manager's current resolved snapshot after `ApplyContext(...)`.
**Rationale**: The manager already owns the authoritative combination of include-graph, inheritance-graph, and preset-model state. Putting the facade here makes its lifecycle explicit and avoids a second object graph with independent refresh rules.
**Alternatives considered**:
- A standalone facade built directly from the low-level objects. Rejected because callers would still need to know refresh ordering and validity boundaries.
- Moving high-level query helpers onto each low-level type. Rejected because cross-cutting queries would remain fragmented.

### 2. Facade answers are projections of existing state, not recomputed interpretations
**Decision**: Query results will be projections of already-resolved model and graph state, including preset availability, resolved fields, unresolved reasons, and workflow diagnostics.
**Rationale**: The facade needs to stay thin. Recomputing effective conditions, resolved fields, or graph relationships inside the facade would duplicate business logic and create inconsistency risk.
**Alternatives considered**:
- Re-evaluating effective state in the facade for convenience. Rejected because it becomes a second resolution layer.
- Returning only raw object references. Rejected because the facade would not simplify anything for common consumers.

### 3. Query scope favors common consumer workflows over full graph generality
**Decision**: The facade will focus on a narrow set of common read-only queries: enumerating files and presets, retrieving per-preset summary/state, inspecting source-file and inheritance/include relationships, and reading diagnostics.
**Rationale**: The gap identified in the review is about ergonomics for common consumers, not about replacing the full graph API. A constrained scope reduces the chance of accidental overreach.
**Alternatives considered**:
- Exposing every graph traversal through the facade. Rejected because it would become a second general-purpose API layer.
- Limiting the facade to preset-name lookup only. Rejected because consumers still need file and relationship context.

### 4. Identifiers stay aligned with existing domain keys
**Decision**: The facade will key file queries by canonical file path and preset queries by preset name plus concrete preset type.
**Rationale**: These are the identifiers already used across the current model and graph layers. Reusing them avoids synthetic IDs leaking into the public surface.
**Alternatives considered**:
- Exposing internal node IDs. Rejected because they are graph-implementation details and unstable across reloads.
- Minting facade-specific handles. Rejected because they add indirection without solving a real problem.

## Risks / Trade-offs

- **Thin facade drift**: There is pressure to add derived logic for every consumer request. -> **Mitigation**: Require the facade to return manager-owned state projections and keep new behavior in the underlying graph/model layers.
- **Naming collisions across preset types**: CMake preset collections are typed, and facade lookup must remain explicit when names overlap by type. -> **Mitigation**: Require typed preset queries and typed summaries.
- **Stale-query expectations**: Consumers may assume query objects auto-refresh when the manager reapplies context. -> **Mitigation**: Define the facade as bound to the manager's current applied state and require a fresh query view after re-application.
- **Over-constraining implementation**: A spec that dictates exact C++ class shapes could make harmless refactors harder. -> **Mitigation**: Specify behavior and ownership boundaries, not exact class names or container choices.
