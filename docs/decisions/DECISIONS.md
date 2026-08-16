# DECISIONS

## DEC-001
- Date / Version: 2026-08-16 / v0.1.0
- Status: ACTIVE
- Decision: Remote-loot proof actions must not call `MoveTo` or `MoveToEx`.
- Reason: movement would make it impossible to distinguish normal built-in pickup behavior from genuine distant server acceptance.

## DEC-002
- Date / Version: 2026-08-16 / v0.1.0
- Status: ACTIVE
- Decision: do not guess unknown runtime method signatures/parameters.
- Reason: this repository is a proof harness; a wrong guessed signature can crash the client and produce misleading evidence.

## DEC-003
- Date / Version: 2026-08-16 / v0.1.0
- Status: ACTIVE
- Decision: direct semantic invocation from the validated message-hook context is allowed only as a one-shot PoC shortcut.
- Consequence: if remote pickup proves viable, the production tool must use a proper ActionGate/MainThread dispatcher/state-proof architecture.
