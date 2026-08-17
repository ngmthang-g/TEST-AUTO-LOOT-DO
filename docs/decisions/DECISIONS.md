# DECISIONS

## DEC-R2-001
- Status: ACTIVE
- Decision: clean v0.2.7 is the only source base for R2; do not inherit old v0.2.8/v0.2.9 code.
- Reason: user reported those versions broadly broken.

## DEC-R2-002
- Status: ACTIVE
- Decision: one global world-coordinate TỌA GD replaces per-CON selector coordinates.
- Consequence: runtime waits for both physical characters at the rendezvous before executing the already-configured shared sequence.

## DEC-R2-003
- Status: ACTIVE
- Decision: inability to stop AutoFight must not permanently prevent a requested movement.
- Consequence: retry twice, travel anyway, retry after map change/destination.

## DEC-R2-004
- Status: ACTIVE / PROTECTED
- Decision: F4 clean-v0.2.7 behavior is byte-protected.
- Reason: explicit user positive runtime feedback.
- Consequence: rehydrate fails if `ToggleGlobalPause()` changes.
