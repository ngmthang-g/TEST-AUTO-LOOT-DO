# PROJECT KNOWLEDGE — v0.2.9

## Critical correction: trade rendezvous owns its own route state
`trainRecoveryPhase` is TRAIN ownership only. v0.2.9 introduces `RuntimeState::tradeRendezvousPhase/tradeRendezvousTick/tradeRendezvousStopAttempts` for trade travel. Never reuse trainRecovery for TỌA GD again. Active MAIN + child are `tradeHeld`, and generic `TickAccount` must not route them.

At TỌA GD, arrival is not enough: StopPath is sent and `TradeAccountStandingAtRendezvous()` must prove life/map/position/AutoFight/AutoPath/riding are authoritative, AutoFight=OFF, AutoPath=OFF, riding=OFF, and position inside rendezvous tolerance. `MaintainTradeRendezvousLock()` gates OpenParty, SelectChild, Sequence and Verify. If AutoPath reappears, StopPath is sent and the next click is withheld. Actual departure from the rendezvous aborts fail-closed.

## Party-open prerequisite
Global `tradePartyPoint_` belongs to MAIN and persists under Global `TradePartyX/Y/W/H/Valid`. Capture path: select any CON -> `TỌA TỔ ĐỘI` -> hover MAIN party/member-panel button -> F8. Runtime order for every round is fixed:
`pair GD LOCK -> MAIN party point -> delay -> per-CON MAIN face/select point -> shared ACC CON workflow`.
Per-CON face coordinates remain profile-specific; only the party-open point is global.

## Main sell inside a pinned child-drain session
The normal Auto Sell phase 8 routes back to the training target. That is forbidden while `tradeTxn_.phase == DrainMainSell`. After bag-space verification, MAIN's sell state must terminate at sellPhase=0 without return-to-train; TickTradeCoordinator then starts dedicated trade-rendezvous travel back to TỌA GD. The child remains GD LOCK at TỌA GD the whole time. Only FinishTrade/AbortTrade releases holds.

## F4 pause reliability
F4 has two input paths: registered global `WM_HOTKEY` and `GetAsyncKeyState(VK_F4)` edge fallback. Both feed `HandlePauseHotkeySignal()` with 350 ms debounce to avoid double toggles. PAUSE gates `TickAccount` and `TickTradeCoordinator`, sends StopPath to RUN accounts, and preserves the current transaction state for resume.

## Preserved v0.2.8 invariants
FULL==0 is only the session-entry gate; the same childPid stays pinned until `freeSlots >= childTargetFreeSlots`. Grouped sequence repeats, round-wide transfer budget, shared MAIN + single global ACC CON workflow, fixed CON1->CON6 priority, DỒN ĐỒ OFF mode, sell-sequence FREEZE ALL and exactly two raw SendInput calls remain mandatory.

---

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
