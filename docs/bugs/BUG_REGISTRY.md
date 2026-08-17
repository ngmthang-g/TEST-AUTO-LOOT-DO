# BUG REGISTRY

## BUG-R2-001 — Trade preparation used training target rather than one trade rendezvous
- Status: FIXED-BUILD / RUNTIME UNTESTED
- First Observed: clean v0.2.7 source inspection
- Root Cause: active trade preparation reused normal train recovery/target semantics.
- Fix: global TỌA GD + dedicated `tradeTravel*` state + both-arrived gate.
- Runtime Verified In: UNKNOWN

## BUG-R2-002 — Unneeded per-CON trade selector coordinate
- Status: REMOVED / RUNTIME UNTESTED
- First Observed: user request v0.2.7-R2
- Fix: remove `tradeSelectPoint`, `TradeSelect*` UI/persistence/runtime.
- Do-Not-Do: do not silently restore it.

## BUG-R2-003 — AutoFight-stop retry could permanently block requested movement
- Status: FIXED-BUILD / RUNTIME UNTESTED
- Root Cause: terminal retry paths entered manual wait.
- Fix: bounded attempts, continue movement, retry around map changes/destination.

## BUG-R2-004 — New rendezvous HOLD could survive OFF/abort without explicit cleanup
- Status: FIXED-BUILD / RUNTIME UNTESTED
- Fix: `ReleaseTradeHolds()` clears all held accounts and trade travel state.

## BUG-R2-005 — First R2 edit accidentally removed untouched clean helper functions
- Status: FIXED-BUILD / RUNTIME UNTESTED
- First Observed: CI #56 BUILD FAILED
- Error: `PeriodicConfirmBusy` and `HandleFightClicks` identifier not found.
- Root Cause: deletion range around old StopAuto1 code was too broad.
- Fix: restore both helper definitions byte-for-byte from exact clean v0.2.7; CI #57 then BUILD/CI PASS.
- Do-Not-Do: do not rewrite unrelated helpers while editing adjacent click slots.
