# FEATURE: REMOTE LOOT POC

## Purpose
Determine whether loot can be interacted with/picked up semantically at distance without the PoC moving the character.

## Current Implementation
- read-only target/context validation;
- runtime loot API discovery;
- nearest ItemPack read probe when runtime signature is supported;
- one-shot distant `ClickToObject` test;
- one-shot distant `PickUpItemFromItemPack(packID,-1,1)` test;
- Càn Khôn Hồ buff 30008009 read probe.

## Current Runtime Status
`RUNTIME UNTESTED`.

## Current Known-Good
None.

## Related Decisions
- `DEC-001`: no movement and no auto-loop in proof phase.
- `DEC-002`: unsupported runtime signatures are logged, never guessed.
- `DEC-003`: PoC hook invocation is not the final production action engine.

## Important APIs / Constants
- `Game.GetNearestItemPack(...)`
- `Game.GetNearbyItemPack(...)`
- `Game.ClickToObject(RoleID)`
- `Game.PickUpItemFromItemPack(itemPackID,-1,1)`
- `Game.HasBuff(30008009)`
- built-in normal loot threshold observed in shipped Lua: distance >100 -> `MoveToEx(...)` before object click.

## Do-Not-Break Rules
1. Remote tests must not call `MoveTo`/`MoveToEx`.
2. One mutable action per manual test.
3. Method invocation success is not pickup success.
4. Need concrete pack/bag/no-movement runtime proof.
5. Crash/disconnect is ambiguous until execution-boundary vs server rejection is separated.

## Open Questions
- Is `RoleID` always the same identifier expected by pickup?
- Is distance rejection enforced by client policy, server, or entitlement/buff state?
- Does Càn Khôn Hồ use the same pickup path?

## Next Diagnostic Step
Run v0.1.0 against a live PID and compare the same distant-pack test with buff 30008009 ABSENT vs PRESENT.
