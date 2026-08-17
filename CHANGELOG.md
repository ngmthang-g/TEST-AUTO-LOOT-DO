# CHANGELOG

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
