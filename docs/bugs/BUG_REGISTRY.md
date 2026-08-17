# BUG REGISTRY

## BUG-R2-001 — Trade preparation used old training target instead of one trade rendezvous
- Status: FIXED-BUILD-PENDING-CI
- First Observed: clean v0.2.7 source inspection
- Last Known-Good: UNKNOWN
- Root Cause: trade preparation reused normal train recovery/target state.
- Fix: dedicated global TỌA GD and `tradeTravel*` state.
- Runtime Verified In: UNKNOWN
- Next Diagnostic Step: full CON while MAIN/CON are on different maps.

## BUG-R2-002 — Per-CON trade selection coordinate is unnecessary configuration
- Status: REMOVED
- First Observed: user request v0.2.7-R2
- Root Cause: clean coordinator required `tradeSelectPoint`.
- Fix: all UI/persistence/capture/runtime references removed.
- Do-Not-Do: do not silently restore per-CON selector coordinates.

## BUG-R2-003 — AutoFight stop retries could block movement forever
- Status: FIXED-BUILD-PENDING-CI
- First Observed: source inspection
- Root Cause: terminal retry states returned “chờ thủ công”.
- Fix: continue route after retry limit; on map change, retry stop and continue.
- Runtime Verified In: UNKNOWN.

## BUG-R2-004 — DỒN ĐỒ OFF could theoretically leave stale future rendezvous hold state after R2 additions
- Status: FIXED-BUILD-PENDING-CI
- Root Cause: new dedicated state needs explicit cleanup.
- Fix: `ReleaseTradeHolds()` scans held accounts and resets rendezvous state.
- Runtime Verified In: UNKNOWN.
