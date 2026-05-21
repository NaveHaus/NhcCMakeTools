## Context

`PresetIncludeGraph` already owns the rules for valid include macro syntax, version-gated `${...}` handling, and the distinction between `UnsupportedMacro` and `MissingMacro`. The current manager flow duplicates part of include expansion inside `PresetsGraph::ApplyContext()`, which breaks that contract at the integration boundary. The architecture review for the preset graph library identifies this as the primary issue for the change and recommends keeping policy in the include graph while leaving discovered-file loading in the manager.

## Goals / Non-Goals

**Goals:**
- Restore a single source of truth for include macro policy.
- Keep file discovery, loading, and iterative orchestration in the manager.
- Make manager-observed diagnostics for unsupported include macros match the include graph behavior.
- Define tasks that follow the repository TDD requirement for manager-level behavior changes.

**Non-Goals:**
- Changing general macro expansion semantics outside include resolution.
- Moving file loading responsibilities out of the manager.
- Changing inheritance, condition, workflow, or simulated-version behavior in this change.

## Decisions

### 1. Delegate include expansion policy to `PresetIncludeGraph`
The manager will treat the include graph as the authoritative component for validating and expanding include strings. `ApplyContext()` should build the per-file local macro context, call the include-graph resolution path, and consume the resolved file paths plus unresolved diagnostics from that layer.

Rationale: this matches the existing capability boundary and removes the dead-code path identified in the review.

Alternatives considered:
- Re-implement include policy in `PresetsGraph::ApplyContext()`. Rejected because it duplicates the rules already owned by `PresetIncludeGraph` and risks further divergence.
- Move discovered-file loading into `PresetIncludeGraph`. Rejected because the manager already owns file loader integration and iterative refresh of preset collections.

### 2. Keep manager orchestration focused on discovery and refresh
The manager should continue to iterate until no new include file paths are discovered, loading newly resolved files and then refreshing typed preset state from successfully processed files.

Rationale: the review direction explicitly keeps discovered-file loading in the manager, and the existing manager spec already models that responsibility well.

Alternatives considered:
- Collapse include resolution and loading into a single component. Rejected because it blurs graph responsibilities and expands the include graph beyond policy and topology.

### 3. Test the integration at the manager boundary
Implementation should start with manager-level RED tests for unsupported include macros observed through `ApplyContext()`, especially cases such as `$env{HOME}`, `${presetName}`, and version-gated `${fileDir}` on v7/v8 files.

Rationale: the bug exists specifically at the integration boundary, so unit coverage must verify the end-to-end manager behavior rather than only the include graph in isolation.

Alternatives considered:
- Rely only on existing include-graph tests. Rejected because those tests already pass while the manager path remains wrong.

## Risks / Trade-offs

- [Risk] Manager integration changes could accidentally suppress iterative discovery of valid include paths.  
  Mitigation: keep tasks separated between delegation refactor and discovery regression coverage.
- [Risk] Delegation may require small API adjustments between manager and include graph.  
  Mitigation: constrain the change to the existing include-resolution flow and avoid broad graph-model refactors.
- [Trade-off] The manager spec becomes more explicit about delegation boundaries.  
  Mitigation: capture only the observable contract needed to prevent policy drift.
