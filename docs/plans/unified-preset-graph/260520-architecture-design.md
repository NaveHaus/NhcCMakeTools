# Unified Preset Graph: Architecture Design

## Context

This document captures the architecture converged during the design
session referenced by branch `claude/cmake-presets-graph-design-6Eo8t`.
It is the authoritative description of the target shape for the
`unified-preset-graph` major feature and is intended to be read in
full by anyone scoping an OpenSpec change against this plan.

The design is the second iteration on the question "what would the
preset graph library look like if we started from scratch?". It
deliberately drops constructs that proved unnecessary during the
design discussion (`MacroNode`, `ContextNode`, `ApplyContext` argument
threading, a `CrossPresetRef` edge kind invented in error) and settles
on a small set of typed entities with explicit ownership.

The design assumes conformance with the CMake 4.3.2 `cmake-presets(7)`
manual. No syntax outside that spec is introduced.

## Goals

- One typed read-only graph that contains every entity the library
  models.
- A single mutation surface for callers, with an explicit
  rebuild trigger.
- Provenance preserved at the field level (inheritance) and the
  edge level (file structure, preset relationships, workflow steps).
- Diagnostics owned by the entity that has the problem.
- Typed placeholders so traversal never returns nullable references
  to "missing" entities.
- No graph entities for macros; macro state is pure input.

## Non-goals

- Tracking which macro keys each field consumed (`consumedKeys`).
  Deferred under YAGNI; can be added without breaking changes.
- Auto-population of host-derived macros (`${hostSystemName}`,
  `${pathListSep}`). Caller input remains authoritative, consistent
  with the decision recorded in the `preset-graph-lib-arch-review`
  plan.
- Backwards compatibility with the existing public API surface in
  this document. Migration sequencing is addressed in the README's
  change list (`migrate-callers-and-retire-legacy-library`).

## Top-level shape

```cpp
class Manager {
public:
    MacroContext& macroContext();             // mutable; not a node
    void setRootFiles(std::span<const fs::path>);
    void setHostInfo(HostInfo);                // generator, hostSystemName, etc.

    void rebuild();                            // explicit; swaps in a fresh Graph
    const Graph& graph() const;                // read-only; stable until next rebuild

private:
    MacroContext context_;
    HostInfo host_;
    std::vector<fs::path> roots_;
    std::unique_ptr<Graph> graph_;             // swapped wholesale by rebuild()
};
```

`MacroContext` is a flat `unordered_map<string, json>` holding only the
external process environment. It is not a node and has no edges. It is
the lookup source for `$penv{}` and the fall-through for `$env{}` when a
preset's `environment` does not define the key.

`HostInfo` carries caller-supplied host-derived values
(`hostSystemName`, optionally `pathListSep`, generator info if not
attached per-preset, etc.). Auto-population from the host system is
explicitly out of scope (see Non-goals).

`Manager::graph()` returns a reference that is valid until the next
`rebuild()` call. `NodeId` values are not stable across rebuilds;
callers must re-look-up by `(kind, name)` after rebuilding.

## Graph

```cpp
class Graph {
public:
    NodeId find(NodeKind, std::string_view name) const;
    const Node& operator[](NodeId) const;
    auto nodesOfKind(NodeKind) const;          // iterator/range
    std::span<const Edge> edgesFrom(NodeId) const;
    std::span<const Edge> edgesTo(NodeId) const;
    auto diagnostics() const;                  // convenience view over all node + edge diagnostics

private:
    std::vector<std::unique_ptr<Node>> nodes_; // index == NodeId.value
    std::vector<Edge> edges_;
    friend class Builder;                      // build pipeline mutates; callers never do
};
```

`NodeId` is a strong typedef around `uint32_t`. Indices are dense for
the lifetime of one `Graph` instance.

## Node taxonomy

One base class, six derived kinds. There is no `MacroNode` and no
`ContextNode`.

```cpp
enum class NodeKind { File, Configure, Build, Test, Package, Workflow };

class Node {
public:
    NodeId id;
    NodeKind kind;
    std::string name;
    std::string type;               // display category; the universal triple's third member
    bool isResolved() const;        // false ⇒ placeholder
    std::span<const Diagnostic> diagnostics() const;
    virtual ~Node() = default;

protected:
    std::vector<Diagnostic> diagnostics_;
};

class FileNode      : public Node { /* path, version, cmakeMinimumRequired, vendor json */ };
class PresetNode    : public Node { /* see below */ };
class ConfigureNode : public PresetNode {};
class BuildNode     : public PresetNode {};
class TestNode      : public PresetNode {};
class PackageNode   : public PresetNode {};
class WorkflowNode  : public Node { /* ordered list of (kind, preset NodeId) steps */ };
```

`PresetNode` carries the resolved field state:

```cpp
struct FieldEntry {
    NodeId provenance;              // self id for own fields; ancestor id for inherited
    boost::json::value unresolved;  // before macro substitution
    boost::json::value resolved;    // after macro substitution
};

struct EnvEntry {
    NodeId provenance;              // which preset in the inherits chain set this key
    boost::json::value unresolved;  // string, or null (null = "unset / remove inherited")
    boost::json::value resolved;
};

class PresetNode : public Node {
    std::unordered_map<std::string, FieldEntry> fields;
    std::unordered_map<std::string, EnvEntry>   environment;
    bool conditionPassed = true;
};
```

The universal triple (`id`, `kind`, `name`, `type`) are typed members
on `Node` and are never stored inside `fields`. They are never
inherited and never macro-substituted.

`environment` is split out from `fields` because:

- It inherits by key-merge rather than wholesale replacement.
- A `null` value removes the inherited key entirely.
- It is the primary lookup source for `$env{}` resolution, so
  treating it identically to other fields would conflate storage with
  resolution semantics.

## Edge taxonomy

```cpp
enum class EdgeKind {
    Defines,         // FileNode → PresetNode / WorkflowNode (declared in this file)
    Includes,        // FileNode → FileNode (`include` array)
    Inherits,        // PresetNode → PresetNode (ordered; same kind)
    Configures,      // BuildNode / TestNode / PackageNode → ConfigureNode (`configurePreset`)
    WorkflowStep,    // WorkflowNode → PresetNode (ordered)
};

struct Edge {
    EdgeKind kind;
    NodeId from;
    NodeId to;
    uint32_t ordinal;               // for ordered kinds; 0 otherwise
    SourceLocation site;            // JSON pointer + file for jump-to-source
    std::vector<Diagnostic> diagnostics;
};
```

No `CrossPresetRef` edge kind. The CMake presets spec defines no
macro-level cross-preset field reference; the only cross-preset
relationships are `inherits`, `configurePreset`, and workflow `steps`,
all already present above.

Macros do not produce edges. Macro resolution leaves its traces in
`FieldEntry`/`EnvEntry` only.

## Diagnostics

```cpp
enum class Severity { Info, Warning, Error };

enum class DiagnosticKind {
    IncludeCycle,
    InheritanceCycle,
    MissingTarget,
    UnsupportedMacro,
    MissingMacro,
    UnresolvedEnvironmentReference,
    EnvironmentReferenceCycle,
    InvariantViolation,
    /* ... */
};

struct Diagnostic {
    Severity severity;
    DiagnosticKind kind;
    std::string message;
    uint32_t cycleId = 0;           // 0 = not cycle-related; nonzero = which cycle
    SourceLocation site;
};
```

Storage is **per-node and per-edge**. Both `Node` and `Edge` own a
`std::vector<Diagnostic>`. `Graph::diagnostics()` provides a flat
iteration over all of them for callers that want a unified list.

`isResolved() == false` always implies at least one `Error`-severity
diagnostic on the node.

## Macro resolution

The library implements the CMake 4.3.2 macro set exactly:

- `${sourceDir}`, `${sourceParentDir}`, `${sourceDirName}`,
  `${fileDir}`, `${hostSystemName}`, `${pathListSep}`,
  `${presetName}`, `${generator}` (where allowed by the spec).
- `$env{name}` — preset environment first, then `MacroContext`.
- `$penv{name}` — `MacroContext` only.
- `$vendor{name}` — pass through verbatim; not flagged as
  unresolved.

`${presetName}` resolves to the **using** preset's name, not the
defining preset's name. Inherited field values are stored in the
descendant's `FieldEntry` with `unresolved` copied from the ancestor
and `resolved` computed in the descendant's context.

### `$env{}` resolution rule

```
resolve($env{X}, presetP):
    if presetP.environment contains key X:
        return value of presetP.environment[X].resolved
    else if MacroContext contains X:
        return MacroContext[X]
    else:
        unresolved → diagnostic
```

### Environment-internal recursion

Values inside `presetP.environment` may themselves contain macros
(`${sourceDir}`, `$env{OTHER}`, `$penv{PATH}`, etc.). Resolution
within the map is on-demand:

- When resolving key `A`, push `A` onto a per-rebuild visiting set,
  recurse into any `$env{B}` references against the same map, then
  pop.
- If `A` is encountered while already on the set, emit an
  `EnvironmentReferenceCycle` diagnostic on the offending preset
  node and leave the affected entries unresolved.

Built-in macros (`${sourceDir}` etc.) and `$penv{}` resolve directly
against caller-supplied state, so they introduce no ordering
constraints with environment resolution.

## Placeholders

Any unresolved reference produces a placeholder of the **expected
derived kind**:

- A workflow step pointing at a missing build preset materialises a
  `BuildNode` placeholder.
- A `configurePreset` field pointing at a missing configure preset
  materialises a `ConfigureNode` placeholder.

Each placeholder:

- Has `isResolved() == false`.
- Carries at least one `Error` diagnostic explaining why it is a
  placeholder.
- Has an empty `fields` map and an empty `environment` map (for
  preset placeholders).

Edges still target the placeholder, so callers traversing edges
never receive a null. The placeholder's `kind` always matches the
expected kind for the edge.

## Cycle handling

Cycles are **preserved as edges** with diagnostic annotations:

- The builder detects each cycle (include or inheritance) and
  assigns it a stable `cycleId` within the current rebuild.
- Each participating edge gains one `Diagnostic` with that
  `cycleId` and the appropriate `DiagnosticKind` (`IncludeCycle` or
  `InheritanceCycle`).
- An edge that participates in multiple cycles receives one
  diagnostic per cycle.
- The edge set is not modified; downstream traversal decides
  whether to follow a flagged edge.

Callers wanting "show me cycle #3" filter edges where
`any(d.cycleId == 3 for d in edge.diagnostics)`.

## Workflow steps

Workflow handling is uniform regardless of resolution state:

- A `WorkflowStep` edge is emitted for every declared step in
  declared order.
- Missing step targets resolve to typed placeholder preset nodes of
  the expected kind.
- The full step sequence is always traversable. Callers check
  `isResolved()` per target.
- There is no "stop at first unresolved" logic anywhere in the
  pipeline.

## Rebuild pipeline

`Manager::rebuild()` constructs a fresh `Graph` in the following
phases and swaps it in atomically at the end. The previous `graph()`
reference is invalidated; callers re-fetch.

1. **Parse.** Walk `roots_` and their `include` trees. Emit
   `FileNode`s, raw `PresetNode`s, raw `WorkflowNode`s,
   `Defines` and `Includes` edges. Detect duplicate names. Detect
   include cycles and annotate participating `Includes` edges.

2. **Inheritance merge.** For each preset, walk `Inherits` edges in
   declared order. For each field present in an ancestor and missing
   in self, copy into self with provenance pointing to the ancestor.
   Apply `environment` key-merge with `null`-means-unset semantics.
   Detect inheritance cycles and annotate participating `Inherits`
   edges. On cycles, leave the affected presets' field maps partial
   and continue.

3. **Macro resolution.**

   3a. *Environment maps.* For each preset, resolve every
       `EnvEntry.unresolved` to `EnvEntry.resolved` with on-demand
       recursion and per-key cycle detection.

   3b. *Other fields.* For each preset, resolve every
       `FieldEntry.unresolved` to `FieldEntry.resolved` using the
       `$env{}` rule above against the already-resolved environment
       map.

   No cross-preset ordering is required at this phase.

4. **Condition evaluation.** Evaluate each preset's `condition`
   field over its resolved fields. Honour null-condition
   non-propagation semantics already implemented in
   `PresetModel::ResolveCondition()`. Store `conditionPassed`.

5. **Workflow wiring.** Emit `WorkflowStep` edges, materialising
   typed placeholders for unknown targets.

6. **Validation.** Kind-specific invariants: a configure preset
   must declare a generator where the spec requires one; a build,
   test, or package preset must reference a configure preset; a
   workflow step target must be the expected kind for its position;
   etc. Emit diagnostics on the appropriate node or edge.

7. **Swap.** Replace `graph_` with the new instance.

## Public API summary

The full caller-facing surface intended by this design is:

```cpp
// Manager
MacroContext& macroContext();
void setRootFiles(std::span<const fs::path>);
void setHostInfo(HostInfo);
void rebuild();
const Graph& graph() const;

// Graph
NodeId find(NodeKind, std::string_view name) const;
const Node& operator[](NodeId) const;
auto nodesOfKind(NodeKind) const;
std::span<const Edge> edgesFrom(NodeId) const;
std::span<const Edge> edgesTo(NodeId) const;
auto diagnostics() const;

// Node
NodeId id;
NodeKind kind;
std::string name;
std::string type;
bool isResolved() const;
std::span<const Diagnostic> diagnostics() const;

// PresetNode (additional)
const std::unordered_map<std::string, FieldEntry>& fields() const;
const std::unordered_map<std::string, EnvEntry>&   environment() const;
bool conditionPassed() const;
```

Mutation is exposed only on `Manager::macroContext()`,
`Manager::setRootFiles()`, and `Manager::setHostInfo()`. Everything
else is read-only between rebuilds.

## What this design buys

- Caller mental model is small: mutate input on `Manager`, call
  `rebuild()`, traverse a read-only `Graph`. No `ApplyContext`
  argument threading, no `MacroNode`, no `ContextNode`.
- Field provenance is precise: every preset field knows the
  `NodeId` that contributed it.
- Placeholders are typed: traversal never gets a null or a base
  `Node*` where a derived kind was expected.
- Edges are pure structure: macro references stay out of edge
  storage, so the edge set scales with file/preset structure, not
  with macro-reference density.
- Rebuild contract is honest: read-only graph plus an explicit
  `rebuild()` means no const accessor secretly performs work.
- Cycle preservation: cycles do not erase information; the caller
  sees them as annotated edges and decides traversal policy.

## Open implementation details

The following are deliberately left for the change-level design:

- Exact storage of `MacroContext` mutation (whole-map assign vs.
  per-key API).
- Exact representation of `SourceLocation`.
- Whether per-node diagnostics live in a `Node` base member or in a
  side map keyed by `NodeId`.
- Whether per-edge diagnostics live on `Edge` or in a side map
  keyed by edge index.
- Whether the parse phase eagerly builds adjacency or constructs it
  lazily on first traversal.
- Migration sequencing relative to `preset-graph-lib-arch-review`.
