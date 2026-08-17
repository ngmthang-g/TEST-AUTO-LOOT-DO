# DECISIONS

## DEC-R2-001
- Status: ACTIVE
- Decision: exact clean v0.2.7 is the only source donor for R2; old v0.2.8/v0.2.9 code is not inherited.
- Reason: user reported those versions broadly broken.

## DEC-R2-002
- Status: ACTIVE
- Decision: one global world-coordinate TỌA GD replaces per-CON selector coordinates and normal-train-target trade preparation.
- Consequence: both physical characters must be snapshot-confirmed at the rendezvous before click sequence execution.

## DEC-R2-003
- Status: ACTIVE
- Decision: inability to stop AutoFight after bounded attempts must not permanently prevent a requested movement.
- Consequence: route proceeds and stop is retried after map change/destination.

## DEC-R2-004
- Status: ACTIVE / PROTECTED
- Decision: clean-v0.2.7 F4 behavior remains byte-identical.
- Context: explicit user positive runtime feedback.
- Consequence: rehydrate fails if protected pause block or F4 registration changes.

## DEC-R2-005
- Status: ACTIVE
- Decision: compiler-detected deletion of unrelated clean helpers is repaired by exact restoration, not redesign.
- Reason: preserve stable v0.2.7 behavior and minimize change surface.
