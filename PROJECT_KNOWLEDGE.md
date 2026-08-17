# PROJECT KNOWLEDGE — v0.2.7

## Trade sequence model: exactly two reusable definitions
Active runtime trade configuration has only:
1. `mainTradeSequence_`: shared MAIN coordinate library.
2. `childTradeSequence_`: one global ordered ACC CON workflow shared by CON1..CON6.

There is no active per-CON workflow selection anymore. `AccountProfile::childTradeSequence` is legacy migration/rollback data only.

## Active child binding
The global child workflow still supports two row targets:
- `target=0`: execute on the **currently active transaction child** (`tradeTxn_.childPid`). This is the CON selected by fixed CON1->CON6 priority.
- `target=1`: execute the referenced `MAIN #n` shared step on MAIN.

This preserves required MAIN/CON interleaving while eliminating duplicate CON1..CON6 workflow data. `CHUYỂN ĐỒ` remains CON-only.

## Shared child persistence
Global section: `ChildTradeSequence`.
`SaveSharedChildTradeSequence()` persists it. `EnsureSharedChildTradeSequence()` performs one-time migration only when the global section does not exist.

Migration priority is deterministic: first non-empty legacy per-CON workflow in CON1->CON6 order; otherwise the legacy combined template. Once the global section exists, even with Count=0, migration will not re-run.

## Editor / REC semantics
`CHUỖI GD ACC CON` is global. A selected CON is only the donor window for REC, coordinate capture and test. Recorded CON coordinates are stored once and used by whichever CON is active in a real trade; BaseW/BaseH scaling remains authoritative.

## Preserved invariants
- DỒN ĐỒ ON/OFF behavior from v0.2.6 is unchanged.
- Fixed CON1->CON6 eligibility priority remains.
- MAIN <=6 sell priority and CON FULL==0 trigger remain in consolidation ON mode.
- All automatic physical clicks still pass through BĐPT / `CoordinatorClick` / REAL INPUT.
- Exactly two `SendInput(` call sites remain (LEFTDOWN/LEFTUP in the raw click function).
- v0.2.5 sell-sequence persistent FREEZE ALL remains unchanged.
