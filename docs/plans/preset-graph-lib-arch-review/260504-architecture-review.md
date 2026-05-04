# Architecture Evaluation: NhcCMakeTools vs OpenSpec Specs
## (Validated against CMake 4.3.2 cmake-presets(7) documentation)

## Context

Read-only evaluation of the `NhcPresetGraph` library against the nine OpenSpec specs in
`openspec/specs/`, cross-checked against the CMake 4.3.2 `cmake-presets(7)` manual. Goal: identify
architectural gaps, spec deviations, and implementation next steps.

---

## CMake 4.3 Spec: Key Facts That Inform the Evaluation

These are direct findings from the CMake 4.3.2 documentation that anchor the issue analysis.

**1. `condition` IS an inherited field.** The CMake spec states presets inherit all fields *except*
`name`, `hidden`, `inherits`, `description`, and `displayName`. `condition` is not in the exception
list — it IS inherited. "Each preset evaluates its own condition" means the inherited condition
expression is evaluated in the **child's** macro context (`${presetName}` = child name, etc.), not
the original parent's context. The NhcCMakeTools OpenSpec condition-inheritance requirements match
the CMake spec exactly.

**2. `condition: null` breaks the inheritance chain.** CMake spec: *"If it is null, the preset is
enabled, but the null condition is not inherited by any presets that may inherit from the preset."*
A child with an absent condition looks up through parents for the first inheritable (non-null)
condition; a parent with null is skipped without propagating the null further.
`PresetModel::ResolveCondition()` implements this correctly.

**3. `${generator}` is explicitly preset-specific.** The CMake spec macro table marks `${presetName}`
and `${generator}` as preset-specific ("derived from the fields inside a preset's definition").
Neither is permitted in include strings at any version. `IsSupportedBraceMacro()` in
`IncludeGraph.cpp` correctly blocks both — but that function is unreachable from the manager
(see Issue 1).

**4. v9+ includes: exact allowed/disallowed list.** Allowed: `$penv{}`, `${sourceDir}`,
`${sourceParentDir}`, `${sourceDirName}`, `${hostSystemName}`, `${fileDir}`, `${dollar}`,
`${pathListSep}`. Disallowed: `$env{}` and all preset-specific macros (`${presetName}`,
`${generator}`). v7/v8: only `$penv{}`.

**5. Version 11 requires CMake 4.3 specifically.** The versions table maps v11 → CMake 4.3.
CMake 4.0/4.1/4.2 support at most v10 (same as 3.31).

**6. Workflow presets do not list `hidden`, `inherits`, `condition`, or `environment`** in their
field specification. The spec workflow preset fields are: `name`, `steps`, `displayName`,
`description`, `vendor`.

---

## Compliance Summary

| Spec | Status | Key Gaps |
|------|--------|----------|
| `preset-graph-core` | ✅ Fully compliant | — |
| `nhc-test-lib` | ✅ Fully compliant | — |
| `preset-file-loader` | ✅ Fully compliant | — |
| `preset-macro-context` | ✅ Fully compliant | — |
| `preset-condition-ast` | ✅ Fully compliant | — |
| `preset-include-graph` | ⚠️ Partial | `ResolveIncludes()` is dead code; macro policy never enforced at manager level |
| `preset-inheritance-graph` | ⚠️ Partial | Condition inheritance not resolved; `EvaluateAvailability()` only checks direct conditions |
| `preset-model` | ⚠️ Partial | `ResolvedPreset` diverges from spec; env not expanded in `RefreshResolvedState()`; `WorkflowPreset` hiding incomplete; `ResolveCondition()` has no cycle guard |
| `preset-graph-manager` | ⚠️ Partial | Include macro policy not enforced; CMake 4.x version mapping bug; stale include graph state on re-apply |

---

## High-Priority Issues

### Issue 1: `PresetIncludeGraph::ResolveIncludes()` Is Dead Code — Include Macro Policy Never Enforced
**Files:** `src/PresetsGraph/IncludeGraph.cpp:154–212`, `src/PresetsGraph/PresetsGraph.cpp:265–321`

`IncludeGraph::ResolveIncludes()` is the sole implementation of the spec-required include macro
policy: version-based `${...}` restriction, `$env{}`/`$vendor{}` rejection, and
`${presetName}`/`${generator}` rejection. `PresetsGraph::ApplyContext()` runs a **parallel include
resolution loop** that never calls `ResolveIncludes()`.

The manager's loop only checks `ExpansionStatus::PartiallyExpanded` → `MissingMacro`. It never
produces `UnsupportedMacro`. Concrete spec violations:

| Include string | Should be | Actually is |
|---|---|---|
| `$env{HOME}` | `UnsupportedMacro` | `MissingMacro` (if HOME absent) or silently expands |
| `$vendor{x}` | `UnsupportedMacro` | Silently left as-is |
| `${presetName}` | `UnsupportedMacro` | `MissingMacro` |
| `${generator}` | `UnsupportedMacro` | `MissingMacro` |
| v7/v8 file using `${fileDir}` | `UnsupportedMacro` | Silently expanded |

**Recommended fix (Option A — preferred):** The include graph layer owns the macro policy. Remove
the duplicated expansion logic from `PresetsGraph::ApplyContext()` and delegate to
`m_IncludeGraph.ResolveIncludes(localContext)` per file (after injecting `fileDir`/`dollar` into
the local context). The iterative discovery loop in the manager stays, but the per-include expansion
and policy enforcement moves into the graph.

**Option B:** Delete `ResolveIncludes()` and lift `IsSupportedBraceMacro()` /
`ContainsUnsupportedSyntax()` into the manager loop. Consolidates logic but erases the layer
boundary.

---

### Issue 2: Condition Inheritance Not Implemented in `PresetInheritanceGraph`
**Files:** `src/PresetsGraph/InheritanceGraph.cpp:83–116`, `src/PresetsGraph/PresetsGraph.cpp:551–582`

CMake spec confirms `condition` is inherited. The NhcCMakeTools OpenSpec requires the inheritance
graph to evaluate the *effective* condition — walking up the `inherits` chain for the first
non-null inheritable condition.

`RefreshInheritanceGraph()` (`PresetsGraph.cpp:571`) copies only the **direct** condition:
```cpp
if(preset->GetCondition() != nullptr) {
    inheritancePayload.ConditionAst = preset->GetCondition()->Clone();
}
```

`EvaluateAvailability()` (`InheritanceGraph.cpp:98`) then treats a missing `ConditionAst` as Active:
```cpp
if(!payload.ConditionAst) {
    payload.Availability = PresetAvailability::Active;  // WRONG: ignores inherited conditions
    continue;
}
```

**Impact:** A preset with no local condition but inheriting from a parent with `condition: false`
is reported `Active` instead of `Disabled`.

**Recommended fix:** In `PresetsGraph::RefreshInheritanceGraph()`, use the already-correct
`PresetModel::ResolveCondition()` to obtain the effective (inherited) condition:
```cpp
const Condition* effective = m_PresetModel.ResolveCondition(preset->GetName());
if(effective != nullptr) {
    inheritancePayload.ConditionAst = effective->Clone();
}
// ExplicitNull resolves to nullptr → ConditionAst is nullptr → Active (correct)
```

**Cycle guard required:** `ResolveCondition()` has no cycle detection (see Issue 5 below).
`RefreshInheritanceGraph()` runs before `InheritanceGraph::Resolve()` (which does cycle detection),
so a cyclic `inherits` chain would cause infinite recursion here. The fix must either:
- Add a `visiting` set guard to `ResolveCondition()`, or
- Reorder so cycle detection runs first and cyclic presets are skipped during condition resolution.

---

### Issue 3 (Bug): CMake 4.x Version Mapping Returns v11 for All 4.x Versions
**File:** `src/PresetsGraph/PresetsGraph.cpp:400–403`

```cpp
if(m_SimulatedVersion.Major >= 4) {
    return 11;  // BUG: v11 requires CMake 4.3; 4.0/4.1/4.2 support only v10
}
```

CMake 4.0/4.1/4.2 are successors to 3.31 (which introduced v10) and support at most v10.
Additionally, the minor-version checks below (`Minor >= 31` etc.) are never reached for any 4.x,
so CMake 4.0 also incorrectly bypasses the v10 path entirely.

**Recommended fix:**
```cpp
if(m_SimulatedVersion.Major > 4
   || (m_SimulatedVersion.Major == 4 && m_SimulatedVersion.Minor >= 3)) {
    return 11;
}
if(m_SimulatedVersion.Major >= 4 || m_SimulatedVersion.Minor >= 31) {
    return 10;
}
// ...rest of 3.x minor checks unchanged
```

---

## Medium-Priority Issues

### Issue 4: `PresetModel::ResolveCondition()` Has No Cycle Guard
**File:** `src/PresetsGraph/PresetModel.cpp:357–381`

`ResolveCondition()` walks the `inherits` chain recursively with no visited-set protection. A
cyclic inheritance chain causes infinite recursion and a stack overflow. Currently the function is
only called from user code (after the InheritanceGraph has already flagged cycles), but the fix
for Issue 2 would call it inside `RefreshInheritanceGraph()` — before cycle detection runs.

**Recommended fix:** Add a `std::unordered_set<std::string>` visiting guard to `ResolveCondition()`
(either as a private overload or by threading it through the call):
```cpp
const Condition* PresetModel::ResolveCondition(
    const std::string& name,
    std::unordered_set<std::string>& visiting) const
{
    if(visiting.count(name)) return nullptr;  // cycle → no effective condition
    visiting.insert(name);
    // ...existing logic...
    visiting.erase(name);
    return inherited;
}
```

---

### Issue 5: Include Graph Nodes Not Reset Between `ApplyContext()` Calls
**File:** `src/PresetsGraph/PresetsGraph.cpp:255–258`

`ApplyContext()` resets `m_LoadedFiles`, `m_PresetModel`, and `m_InheritanceGraph` but does **not**
reset `m_IncludeGraph`. File nodes carry `IsUnresolved`/`Reason` forward. `TryLoadFileNode()` sets
these flags but never clears them before a re-load attempt.

**Scenario:** A file missing on the first call (marked `FileDoesNotExist`) is created, then
`ApplyContext()` is re-called. The file loads successfully but the node still shows `IsUnresolved`.
`ComputeState()` reports `Unresolved` even though the graph is clean.

**Recommended fix:** At the start of `TryLoadFileNode()`, clear the node's unresolved state before
attempting the load:
```cpp
auto& payload = m_IncludeGraph.GetFilePayload(nodeId);
payload.IsUnresolved = false;
payload.Reason = std::nullopt;
```
Or add a `ClearUnresolved()` method to `FilePayload` mirroring `Preset::ClearUnresolved()`.

---

### Issue 6: `WorkflowPreset` API Hiding Is Incomplete
**File:** `src/PresetsGraph/PresetModel.h:173–183`

CMake spec workflow presets expose only `name`, `steps`, `displayName`, `description`, `vendor`.
The NhcCMakeTools OpenSpec states: "the typed API SHALL expose only `name` and `steps`."

Currently hidden via `private using` in `WorkflowPreset`:
`GetCondition`, `SetCondition`, `GetEnvironment`, `SetEnvironment`, `GetHidden`, `SetHidden`,
`GetInherits`, `SetInherits`

**Not hidden but should be:** `GetConditionState`, `SetConditionExplicitNull`, `ClearCondition`

**Recommended fix:** Add three more `private using` declarations:
```cpp
using Preset::GetConditionState;
using Preset::SetConditionExplicitNull;
using Preset::ClearCondition;
```

---

### Issue 7: `ResolvedPreset` Struct Diverges from Spec Guidance
**File:** `src/PresetsGraph/PresetModel.h:186–197`

The `preset-model` OpenSpec: *"The system SHALL NOT require a separate model-managed `ResolvedPreset`
or `RawResolvedPreset` object to represent resolved state."* The spec-compliant path is
`RefreshResolvedState()` + `GetResolvedFields()` — per-field status stored on the preset.

`ResolvePreset()` returns a `ResolvedPreset` struct that lacks per-field status, omits most fields,
and duplicates the resolution logic already inside `ResolveMergedFields()`. The internal
`MergedPresetFields` type is private and drives both paths.

**Recommended fix:** Deprecate `ResolvePreset()` / `ResolvedPreset` as public API. Migrate callers
to `RefreshResolvedState()` + `GetResolvedFields()`. Keep `ResolveMergedFields()` as the shared
internal implementation.

---

### Issue 8: Environment Values Not Expanded in `RefreshResolvedState()`
**File:** `src/PresetsGraph/PresetModel.cpp:397–411`

`RefreshResolvedState()` expands only four scalar fields (`generator`, `installDir`, `binaryDir`,
`toolchainFile`). The `environment` block from raw JSON is stored with `FullyResolved` status
without expanding macro references inside values. An env entry containing `$env{MISSING}` would be
marked fully resolved even though it isn't.

**Recommended fix:** After `ResolveMergedFields()`, expand each `RawEnvironment` entry using the
local context (as `ResolvePreset()` already does) and store each key-value pair as a separate
`ResolvedField` with its correct resolution status.

---

## Minor Issues

### Issue 9: `IsScalarPresetField()` Coverage Is Narrow
**File:** `src/PresetsGraph/PresetModel.cpp:82–86`

Only `generator`, `installDir`, `binaryDir`, `toolchainFile` are treated as expandable string
scalars. Other CMake preset string fields (`cmakeGeneratorPlatform`, `cmakeGeneratorToolset`,
`toolsetsPath`, etc.) are stored as `FullyResolved` raw JSON. Acceptable per the OpenSpec
("all other fields MAY be retained in raw JSON") but limits resolved-state usefulness.

**Recommended fix:** Extend incrementally as field coverage grows.

---

## Implementation Next Steps

### Next Step 1 (Critical): Wire Include Macro Policy (fixes Issue 1)
Call `m_IncludeGraph.ResolveIncludes(localContext)` from the manager and remove the duplicate
expansion loop. This is the highest-impact change for `UnsupportedMacro` correctness.

### Next Step 2 (Critical): Effective Condition in `RefreshInheritanceGraph()` (fixes Issues 2 + 4)
Add cycle guard to `ResolveCondition()` first, then change `RefreshInheritanceGraph()` to use
`m_PresetModel.ResolveCondition()` to resolve the effective (inherited) condition for each preset
payload. Required for correct `Active`/`Disabled` availability on presets that inherit conditions.

### Next Step 3 (Bug Fix): Correct CMake 4.x Version Mapping (fixes Issue 3)
Two-line change as shown above. Required for correct diagnostics when simulating CMake 4.0/4.1/4.2.

### Next Step 4: File Node State Reset on Re-apply (fixes Issue 5)
Clear `IsUnresolved`/`Reason` at the start of `TryLoadFileNode()`. Makes `ApplyContext()`
idempotent across multiple calls.

### Next Step 5: Complete `WorkflowPreset` API Hiding (fixes Issue 6)
Three additional `private using` declarations.

### Next Step 6: Auto-Populate Host-Derived Macros
The manager currently injects only `${fileDir}` and `${dollar}` into the local context for include
expansion. The CMake spec's v9+ macro set also includes host-derived macros the library can
populate deterministically without caller input:

- **`${hostSystemName}`** — equivalent to `CMAKE_HOST_SYSTEM_NAME`. Should default to the same
  string CMake would produce on the caller's system. In C++, `CMAKE_HOST_SYSTEM_NAME` is a
  compile-time constant injected by CMake itself, so the library can use it directly when built
  with CMake (e.g., `"Windows"`, `"Linux"`, `"Darwin"`). Callers may still override this value
  for cross-compilation scenarios.
- **`${pathListSep}`** — derived from `${hostSystemName}`: `";"` when `hostSystemName` is
  `"Windows"`, `":"` otherwise. Because it is fully determined by `hostSystemName`, it should
  never be set independently; the library computes it automatically once `hostSystemName` is known.

The library should auto-populate both at `ApplyContext()` time. Macros like `${sourceDir}`,
`${sourceParentDir}`, `${sourceDirName}` remain caller-injected since the library has no
knowledge of the project source tree.

Concrete change: add a `PresetsGraph::PopulateHostMacros(MacroContext&)` static helper (or
populate directly in `ApplyContext()`) that sets `hostSystemName` from the compile-time constant
and derives `pathListSep` from it. Callers who need cross-compilation behaviour can override
`hostSystemName` in their `MacroContext` before calling `ApplyContext()`, and `pathListSep` would
need to be recomputed or also overridden.

### Next Step 7: Extend `IsScalarPresetField()` Coverage (fixes Issue 9)
Add additional CMake preset scalar field names as field coverage grows.

### Next Step 8: Deprecate `ResolvePreset()` / `ResolvedPreset` (fixes Issue 7)
Migrate callers to `RefreshResolvedState()` + `GetResolvedFields()`.

### Next Step 9: Expand Environment in `RefreshResolvedState()` (fixes Issue 8)
Store per-key expanded environment entries with correct resolution status.

### Next Step 10: Facade / Integration Entry Point
Expose a lightweight facade over the three sub-objects (`GetIncludeGraph()`,
`GetInheritanceGraph()`, `GetPresetModel()`) for the common use cases: list visible presets,
query a resolved field value, check overall state.

### Next Step 11: Surface Workflow Diagnostics on the Preset
Currently `GetWorkflowDiagnostics()` is a separate list on `PresetsGraph`. Consider also calling
`workflow->MarkUnresolved(...)` so consumers query a single object rather than two.

---

## Verification (No Code Changes Proposed)

| Issue | How to verify |
|-------|--------------|
| Issue 1 (dead `ResolveIncludes`) | Add a `GraphManagerTests` scenario with `$env{HOME}` in an include string; confirm `UnsupportedMacro` is **not** currently reported. |
| Issue 2 (condition inheritance) | Add a scenario: C has no condition and inherits from P0 with `condition: false`. Assert `InheritanceGraph` marks C `Disabled`. Currently reports `Active`. |
| Issue 3 (version mapping) | Construct `PresetsGraph` with `CMakeVersion{4, 0, 0}`. Load a file with `"version": 11`. `IsVersionSupported(11)` currently returns `true`; should return `false`. |
| Issue 4 (cycle guard) | Call `ResolveCondition()` on a preset that participates in a direct cycle. Currently stack-overflows. |
| Issue 5 (stale state) | Load a missing file → `FileDoesNotExist`. Create the file. Call `ApplyContext()` again. `ComputeState()` currently still returns `Unresolved`. |