# CHANGELOG

## v0.2.7-R1 — clean rebase, requested-only
- Rebased directly from exact v0.2.7 head `1308b28bd38fb044b9fceed3671820e45fb2cd23`; v0.2.8/v0.2.9 are intentionally excluded and no code from them is reused.
- Added multi-row group repeat to the global `CHUỖI GD ACC CON`; existing single-row repeat remains.
- Added per-CON `TradeDrainFreeSlots`: child must be FULL=0 only to enter a session, then the same child remains pinned until its configured free-slot target is reached or MAIN reaches its existing <=6 sell threshold.
- Added one user-captured global `TỌA ĐỘ GIAO DỊCH`. MAIN + selected child first cancel normal map/train AutoPath and AutoFight, then both use the existing robust route layer to that point while trade HOLD blocks normal TickAccount routing.
- After session completion, release HOLD and return control to the untouched v0.2.7 auto map/train/sell flow.
- Changed `AUTO BÁN ĐỒ KHI TÚI FULL` UI from checkbox to `AUTO BÁN FULL: BẬT/TẮT` pushbutton while preserving `profile.enableSell` semantics and sell scheduler behavior.
- Preserved shared MAIN/shared ACC-CON model, fixed CON1->CON6 priority, BĐPT REAL INPUT, exactly two SendInput call sites, REC, sell FREEZE ALL, DỒN ĐỒ OFF independent mode, route/revive and other stable v0.2.7 behavior.

## v0.2.7
- Replaced per-CON trade workflows with one global `CHUỖI GD ACC CON` shared by CON1..CON6.
- Runtime now binds every CON-targeted row to the active transaction child selected by BĐPT.
- Kept `CHUỖI GD MAIN` as the shared MAIN coordinate library; shared CON workflow can still reference MAIN #n so interleaved MAIN/CON trade order is preserved.
- `CHUYỂN ĐỒ` remains active-CON-only.
- REC/LẤY TỌA/TEST for the shared child workflow use the currently selected CON as a donor window; saved coordinates then apply to every CON.
- Added one-time deterministic migration from old per-CON workflows, prioritizing CON1 -> CON6.
- Moved the shared ACC CON button to the MAIN-sequence button slot for CON roles so it no longer overlaps `CHUỖI CLICK BÁN ĐỒ`.
- Preserved v0.2.6 DỒN ĐỒ toggle/independent auto-train mode and all BĐPT/FREEZE invariants.

## v0.2.6
- Added `DỒN ĐỒ: BẬT/TẮT` push-button mode switch.
- DỒN ĐỒ OFF disables MAIN↔CON transactions and makes every running account auto-train/sell independently on FULL = 0.
- MAIN <=6 threshold is used only while DỒN ĐỒ is ON.
- Turning DỒN ĐỒ OFF aborts an active trade and releases trade holds.
- `CHUỖI CLICK BÁN ĐỒ` is accessible for every selected account.
- Added `LẤY CHUỖI CỦA ACC...` to clone the complete sell-click sequence from another scanned account.
- Preserved persistent FREEZE ALL across the whole sell click sequence.
