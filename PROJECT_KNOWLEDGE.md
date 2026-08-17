# PROJECT KNOWLEDGE — v0.2.8

## Core trade-session invariant
`FreeBagSpace == 0` on a child is an **entry gate only**. It is NOT re-evaluated as a requirement before every subsequent round.

When BĐPT selects CONn, `tradeTxn_.childPid` is pinned for the whole drain session. The session ends only when the same child reaches its persisted per-account `TradeDrainTargetFreeSlots` target, or a safety/error abort occurs.

## Safe rendezvous before trade
Global `tradeRendezvous_` stores one user-captured Map/X/Y (`TradeRendezvousMap/X/Y/Valid/Tolerance`). Before each real trade round:
1. hold MAIN + selected child,
2. stop AutoFight on both using the existing two stop-click points through BĐPT,
3. route both with donor v1.5.9 robust travel to the same rendezvous,
4. require both alive/map-ready and standing at rendezvous with AutoFight/AutoPath/riding off,
5. only then MAIN selects the child and the shared ACC CON workflow runs.

Death/map instability/state loss during selection, sequence or verify is fail-closed and aborts the session instead of continuing clicks.

## Persistent child drain
Each `AccountProfile` has `tradeDrainTargetFreeSlots` (1..90, default 6).
- Start eligibility remains exact FULL: child `freeBagSpace == 0`.
- On start, target is copied into `tradeTxn_.childTargetFreeSlots` so editing the UI mid-session cannot change the active contract.
- After each workflow round, `VerifyChild` waits for bag space to stabilize and checks `freeBagSpace >= childTargetFreeSlots`.
- Below target: same child remains held and another round starts even though it is no longer FULL.
- MAIN sell threshold still has absolute safety priority. After at least one round, if MAIN reaches `<= mainSellThreshold_`, phase `DrainMainSell` runs the existing MAIN sell workflow while the child remains held; then MAIN returns to rendezvous and the same child session continues.

## Grouped mini-sequences
`TradeSequenceStep` adds `groupId` and `groupRepeat`.
- Positive equal `groupId` on contiguous rows defines a mini-sequence.
- Group requires at least 2 contiguous rows and repeat >=2.
- The group can interleave active-CON rows and `MAIN #n` references.
- Existing per-row `repeat` is evaluated first; when the last row of the group finishes, `AdvanceTradeSequenceIndex()` loops to the group start until `groupRepeat` is satisfied.
- `NormalizeTradeGroups()` repairs IDs after editor mutations and prevents accidental non-contiguous groups.
- Copy/paste remaps pasted group IDs so they cannot merge into an existing group accidentally.
- A shared ACC CON workflow must contain at least one `CHUYỂN ĐỒ` row; otherwise persistent drain is rejected.

## MAIN capacity guard across grouped rows
A trade round snapshots one safe transfer-click budget from MAIN free space above `mainSellThreshold_`. `roundTransferClicks` counts **all** CON `CHUYỂN ĐỒ` clicks across row repeats and group repeats. Once budget is exhausted, remaining transfer clicks are skipped while confirmation/other workflow steps continue. This prevents several transfer rows/groups from independently consuming the same budget.

## Preserved invariants
- Active trade configuration remains exactly two reusable definitions: shared `mainTradeSequence_` and one global `childTradeSequence_` for whichever CON is active.
- Fixed CON1 -> CON6 priority remains.
- DỒN ĐỒ OFF remains independent auto-train/sell mode.
- All automatic physical clicks use BĐPT `CoordinatorClick` and REAL INPUT; exactly two `SendInput(` call sites remain in the raw LEFTDOWN/LEFTUP function.
- Persistent FREEZE ALL across the whole sell-click sequence remains unchanged.
