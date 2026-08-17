# PROJECT KNOWLEDGE — v0.2.7-R2

## Project Identity
- Name: Thần Long Item Consolidator / Auto dồn đồ
- Repository: `ngmthang-g/TESTauto-don-do-acc-chinh`
- Development branch: `agent/item-consolidator-v0.2.7-r2-rendezvous-groups`
- Current version: v0.2.7-R2
- Source base: exact clean v0.2.7 commit `1308b28bd38fb044b9fceed3671820e45fb2cd23`
- Base controller SHA256: `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`
- Final controller SHA256: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`

## Current State
### Runtime-confirmed working / protected
- **F4 pause** is explicitly user-confirmed useful and is protected. R2 keeps clean-v0.2.7 `ToggleGlobalPause()` byte-identical and keeps the single F4 `RegisterHotKey` line.

### Built but runtime-untested
- R2 trade rendezvous, grouped mini-sequence, five-click model, per-CON coordinate removal and AutoFight-stop fallback require live runtime validation.
- Final CI #57 / run `32043612053`: **CI PASS / BUILD PASS**. This does not establish live runtime success.

### Known regressions / failed build history
- CI #55: transport EOL mismatch after Windows `git apply`; source logic not implicated. Wrapper now canonicalizes LF before checksum.
- CI #56: compile exposed accidental deletion of clean-v0.2.7 `PeriodicConfirmBusy()` and `HandleFightClicks()` while deleting adjacent StopAuto1 code. Both helpers were restored byte-for-byte from clean v0.2.7.

## Architecture / Hard Rules
- Clean v0.2.7 is the only R2 source donor. **Do not import old v0.2.8/v0.2.9 code.**
- Exactly two active reusable trade definitions remain: `mainTradeSequence_` and global `childTradeSequence_`.
- MAIN `freeBagSpace <= 6` has sell priority in consolidation mode.
- CON FULL exactly (`freeBagSpace == 0`) is the **entry gate only** for selecting a transaction child.
- Eligible CON priority remains fixed CON1→CON6.
- BĐPT owns all automatic physical clicks; raw REAL INPUT remains the only automatic click backend.
- Exactly 2 `SendInput(` call sites must remain.
- No `tradeGlobalFreeze_`; no `roundRobinCursor_`.
- v0.2.5 persistent FREEZE ALL across actual sell-click sequence remains.
- F4 is protected from redesign without new explicit user request/evidence.

## R2 Trade Rendezvous
- Global `tradeRendezvous_` stores Map/X/Y; default tolerance 120.
- On FULL CON selection, MAIN+CON are `tradeHeld` immediately and old training AutoPath is stopped.
- Dedicated `tradeTravel*` state controls rendezvous movement; normal train target is not reused as the trade target.
- First arrival waits and is kept off old-map AutoPath.
- Sequence begins only after both snapshots confirm rendezvous readiness.
- Map transition during fallback movement waits/retries instead of aborting solely because AutoFight was not stopped.
- OFF/abort calls `ReleaseTradeHolds()` to clear all held participants and rendezvous state.

## Trade Sequence Groups
- `TradeSequenceStep` gains `groupId` / `groupRepeat`.
- One or more consecutive rows can be grouped; group repeat 1..999.
- Existing row repeat remains nested inside each group cycle.
- Copy/paste/move/delete normalize or remap groups.

## Five Click Model
Per-account points are now exactly:
1. XÁC NHẬN RA MAP
2. ĐẦU THAI
3. AUTO
4. ĐÁNH QUÁI
5. DỪNG AUTO 2

Old `DỪNG AUTO 1` is removed because it is the same physical point as `AUTO`.

## Failed / Unsafe Mechanisms
- Do not restore per-CON `tradeSelectPoint`/`TradeSelect*` configuration.
- Do not let repeated AutoFight-stop failure deadlock requested movement forever.
- Do not delete/replace clean helper functions merely because they are adjacent to changed click code.
- Do not change F4 while working on trade/rendezvous features.

## Knowledge Index
- Version: `docs/history/VERSION_v0.2.7-R2.md`
- Feature: `docs/features/TRADE_RENDEZVOUS.md`
- Bugs: `docs/bugs/BUG_REGISTRY.md`
- Decisions: `docs/decisions/DECISIONS.md`
- Evidence: `docs/evidence/EVIDENCE_REGISTRY.md`
- Protocol application: `docs/protocol/PROTOCOL_V2_APPLIED.md`; the complete user-supplied Protocol V2 is also included verbatim in the source release archive.
