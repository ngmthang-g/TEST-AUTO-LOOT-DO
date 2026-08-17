# PROJECT KNOWLEDGE — v0.2.7-R3

## Project Identity
- Name: Thần Long Item Consolidator / Auto dồn đồ
- Repository: `ngmthang-g/TESTauto-don-do-acc-chinh`
- Development branch: `agent/item-consolidator-v0.2.7-r3-multidelete-only`
- Current version: v0.2.7-R3
- Direct runtime source base: exact v0.2.7-R2 controller SHA256 `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`
- Final controller SHA256: `a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf`

## R3 Scope — Strictly One Runtime Function
User explicitly ordered all other code/features frozen. R3 changes only `DeleteTradeSequenceRow()` in the trade click-sequence editor:
- `- XÓA` now deletes all selected rows from the existing Ctrl/Shift multi-selection.
- Rows are erased in reverse order to preserve indices.
- Multiple deleted MAIN rows repair shared ACC CON `MAIN #n` references: deleted refs become invalid; surviving refs shift to the correct new MAIN index.
- ACC CON group normalization remains the already-existing behavior after deletion.
- One-row delete still works.

## Protected / Unchanged From R2
- **F4 pause/hotkey is protected and must not change.** R3 wrapper compares the F4 block before/after the R3 patch.
- DỒN ĐỒ ON/OFF.
- Global TỌA GD rendezvous and both-arrived wait.
- Group mini-sequence repeat.
- Five-click model.
- AutoFight-stop fallback movement.
- MAIN `freeBagSpace <= 6` sell priority.
- CON FULL exactly (`freeBagSpace == 0`) entry gate and fixed CON1→CON6 priority.
- BĐPT/REAL INPUT remains the only automatic physical click backend; exactly 2 raw `SendInput(` call sites.
- Persistent sell-sequence FREEZE ALL.
- REC, train, route, death/revive and other donor/core logic.
- No old v0.2.8/v0.2.9 code imported.

## Build / Runtime State
- Local patch apply test: PASS; R2 exact source + R3 patch is byte-identical to final R3 controller.
- Exhaustive index/remap model test for multi-delete MAIN references: PASS for all delete subsets across test sizes 1..8.
- CI #60 / run `32048547405`: failed only on Windows patch EOL checksum before compile; wrapper now canonicalizes patch transport to LF.
- CI #61 / run `32048648947`: **CI PASS / BUILD PASS**; all build/tests/static audits/artifact upload passed.
- Runtime: **RUNTIME UNTESTED** until user verifies live multi-delete.

## Source Identity
- R2 controller SHA256: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`
- R3 controller SHA256: `a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf`
- R3 patch SHA256: `3b013821934c882cce8dc755894f66ab835feec394d3433015127a8792fc2136`

## Knowledge Index
- R3 version history: `docs/history/VERSION_v0.2.7-R3.md`
- R2 version history: `docs/history/VERSION_v0.2.7-R2.md`
- R2 rendezvous feature: `docs/features/TRADE_RENDEZVOUS.md`
- Bugs: `docs/bugs/BUG_REGISTRY.md`
- Decisions: `docs/decisions/DECISIONS.md`
- Evidence: `docs/evidence/EVIDENCE_REGISTRY.md`
