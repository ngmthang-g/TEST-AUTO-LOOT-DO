# VERSION v0.2.1 — exact trade coordination

## User-authoritative logic
- MAIN and CON use the same v1.5.9 route/death/revive process.
- MAIN free bag <= 6 means immediate Auto Sell priority. Trade may start only above the threshold.
- CON is eligible only when its bag is FULL (0 free slots).
- Multiple FULL children use fixed priority CON1 -> CON2 -> ... -> CON6.
- Preparation is sequential: MAIN stops AutoFight and returns to its selected train target, then the chosen CON does the same.
- When both are standing still at train, MAIN runs the child-slot-specific target/select macro and the trade click chain begins.
- During the actual trade macro chain, every other gameplay automation is frozen globally. Read-only snapshots still refresh.
- Death/revive/map-confirm/route/sell/fight do not preempt the trade chain. They resume after MAIN final confirm releases the global freeze.
- The transaction no longer forces start_train; donor v1.5.9 handles the post-trade state in its normal priority order.

## Correctness guards
- Dynamic give-click cap aims not to intentionally cross below the MAIN sell threshold.
- Macro files remain fail-closed while `UNCONFIGURED` is present.
- Background PostMessage remains the runtime click backend.
- Added pure trade scheduler tests for threshold and fixed child priority.
