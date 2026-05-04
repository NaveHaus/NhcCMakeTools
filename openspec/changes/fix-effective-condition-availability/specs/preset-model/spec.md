## ADDED Requirements

### Requirement: Cycle-Safe Effective Condition Resolution
The preset model SHALL resolve a preset's effective condition using standard `inherits` precedence without recursing indefinitely through cyclic inherits chains.

Effective-condition lookup SHALL:
- return the preset's local evaluable condition when one is present;
- treat local `condition: null` as clearing the current preset's inherited condition;
- skip inheriting an explicit `null` condition from an ancestor; and
- stop traversing any inherits path that re-enters a preset already being visited.

If every reachable inherits path is terminated by explicit `null`, missing parents, or a detected cycle before producing an evaluable condition, the effective condition SHALL be absent.

#### Scenario: ResolveCondition returns an inherited evaluable condition
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and does not define a local condition
- **WHEN** the preset model resolves the effective condition for `C`
- **THEN** the result is the evaluable condition inherited from `P0`

#### Scenario: ResolveCondition skips explicit null when traversing descendants
- **GIVEN** preset `P0` has condition `false`
- **AND** preset `C` inherits from [`P0`] and defines `condition: null`
- **AND** preset `G` inherits from [`C`] and does not define `condition`
- **WHEN** the preset model resolves the effective condition for `G`
- **THEN** the result is absent

#### Scenario: ResolveCondition terminates safely on an inherits cycle
- **GIVEN** preset `A` inherits from [`B`]
- **AND** preset `B` inherits from [`A`]
- **AND** neither preset defines a local evaluable condition
- **WHEN** the preset model resolves the effective condition for `A`
- **THEN** the lookup completes without unbounded recursion
- **AND** the result is absent
