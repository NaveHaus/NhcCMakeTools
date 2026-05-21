## Context

The architecture review identified two coupled defects in condition handling for preset availability. First, inheritance-graph availability is computed from each preset's direct condition payload, so a child preset without a local condition is treated as active even when it inherits a disabling condition from a parent. Second, the obvious source of truth for the effective condition, `PresetModel::ResolveCondition()`, is not safe to call during graph refresh because it can recurse indefinitely through cyclic `inherits` chains before inheritance-cycle diagnostics are computed.

This change is limited to artifact definitions for a narrow implementation plan. It does not broaden preset condition semantics beyond the current OpenSpec and CMake-aligned behavior.

## Goals / Non-Goals

**Goals:**
- Define a cycle-safe effective-condition lookup contract in `preset-model`.
- Define inheritance-graph availability in terms of the effective inherited condition rather than the direct local condition.
- Preserve existing explicit-`null` behavior, where `condition: null` clears the current preset's inherited condition without propagating that null to descendants.

**Non-Goals:**
- Changing general inheritance-cycle diagnostics or graph topology rules.
- Changing condition AST parsing or evaluation semantics.
- Refactoring unrelated preset refresh, include resolution, or resolved-state behavior.

## Decisions

### Use `PresetModel::ResolveCondition()` as the single effective-condition source

`PresetModel` already owns the condition inheritance semantics, including explicit `condition: null` chain-breaking behavior. Recomputing those rules inside the inheritance graph would duplicate logic and increase the odds of divergence.

Alternative considered:
- Reimplement effective-condition lookup inside `RefreshInheritanceGraph()`. Rejected because it splits one semantic contract across two layers and would need to duplicate explicit-null handling.

### Add cycle protection inside effective-condition resolution

Effective-condition lookup must tolerate cyclic `inherits` traversal because graph refresh populates inheritance payloads before cycle diagnostics finalize. The resolver should track a visiting set keyed by preset name and terminate lookup when it re-enters a preset already on the active path.

For this change, a detected cycle during condition lookup resolves to "no effective condition available from that path" rather than throwing or marking the preset directly. The inheritance graph remains responsible for the authoritative `InheritanceCycle` unresolved diagnostic.

Alternatives considered:
- Run inheritance-cycle detection before publishing condition payloads. Rejected because the refresh sequence already depends on payload construction before full graph resolution.
- Throw or return a separate cycle error from `ResolveCondition()`. Rejected because the call site only needs a safe effective-condition lookup, while graph diagnostics already carry the cycle-reporting contract.

### Publish effective conditions into inheritance payloads

Inheritance payload construction should clone the effective condition returned by the model and store that payload for availability evaluation. When effective lookup returns no condition, availability rules continue to treat the preset as active unless hidden or vendor-disabled. This keeps explicit `condition: null` behavior intact and also avoids refresh-time recursion failures on cyclic input.

Alternative considered:
- Leave payloads direct-only and teach availability evaluation to climb parents dynamically. Rejected because it makes availability depend on graph traversal at evaluation time and duplicates inheritance precedence rules outside the model.

## Risks / Trade-offs

- [Cycle-safe lookup can mask the immediate source of a cyclic path during refresh] -> Keep cycle diagnostics in the inheritance graph unchanged and treat model lookup only as a safe value query.
- [Two layers now cooperate on condition behavior] -> Keep the boundary crisp: `PresetModel` resolves the effective condition; `PresetInheritanceGraph` evaluates availability from the provided payload.
- [Spec wording could drift from current behavior for explicit `null`] -> Reuse the existing explicit-null semantics in both the design and delta specs instead of redefining them.

## Migration Plan

No external migration is required. The implementation should land behind the existing public APIs and be verified with targeted tests first, then the existing workflow preset should continue to pass full graph verification unchanged.

## Open Questions

None.
