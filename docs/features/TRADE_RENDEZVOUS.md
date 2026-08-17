# FEATURE: TRADE_RENDEZVOUS

## Purpose
When a CON becomes FULL, temporarily take MAIN + that CON out of ordinary training, bring both to one common user-selected world coordinate, and start the already-existing click trade sequence only after both are confirmed there.

## Current Implementation
- Global `tradeRendezvous_` Map/X/Y, captured by `TỌA GD • LẤY`.
- FULL `freeBagSpace == 0` is an entry gate only.
- Both participants become `tradeHeld` immediately.
- Old AutoPath is stopped; bounded AutoFight-stop attempts run using merged `AUTO` + `DỪNG AUTO 2`.
- Dedicated `tradeTravel*` state routes both to TỌA GD.
- First arrival remains held; any reactivated AutoPath is stopped.
- Sequence begins only when both participants are `tradeTravelReady` and position/map checks pass.
- Map transition while fallback travel is active causes wait/retry, not automatic transaction destruction.

## Current Runtime Status
**RUNTIME UNTESTED.** Build/CI status belongs in EVIDENCE_REGISTRY and does not upgrade runtime status.

## Related REQ / BUG / DEC
REQ-R2-001..008; BUG-R2-001..005; DEC-R2-001..004.

## Version Timeline
### clean v0.2.7
- Trade preparation depended on ordinary training target/recovery state and per-CON selector coordinate.

### v0.2.7-R2
- One common rendezvous replaces train-target preparation and per-CON selection coordinate.
- Grouped mini-sequence loop added without replacing the shared sequence model.
- AutoFight stop becomes bounded/non-deadlocking for movement.

## Important State / Constants
- Rendezvous tolerance default: 120.
- Fixed child priority: CON1→CON6.
- MAIN <=6 free slots keeps sell priority at session start.
- First-arrival participant remains `tradeHeld` and must have AutoPath off.

## Do-Not-Break Rules
- Do not use normal train target as the transaction rendezvous.
- Do not restore per-CON selector coordinates.
- Do not run trade clicks until both snapshots prove arrival.
- Do not let repeated AutoFight-stop failure block movement forever.
- Do not bypass BĐPT/REAL INPUT for physical clicks.
- Do not alter F4 as part of rendezvous work.

## Next Diagnostic Step
Live-test one FULL CON with MAIN/CON on different maps and intentionally let one arrive much earlier; then test an AutoFight-stop failure during map travel.
