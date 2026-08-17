# CHANGELOG

## v0.2.8
- Added one global user-captured `TỌA ĐỘ GIAO DỊCH`; MAIN and selected CON both stop AutoFight and route there before a trade round starts.
- Added fail-closed death/map/state checks around rendezvous, selection, trade sequence and child verification.
- Added grouped mini-sequence repeats to the global `CHUỖI GD ACC CON`: select consecutive rows, set group repeat, group/ungroup them, and repeat the whole ordered group before continuing the outer workflow.
- Groups may contain active-CON rows and MAIN references; existing per-row repeat remains supported.
- Added per-CON `DỪNG GD KHI CON TRỐNG ≥ N ô` setting.
- Kept exact FULL=0 as the only **start** condition, but pinned the selected child for the session and continued trading it until its own free-slot target is reached.
- Added `VerifyChild` bag-stability phase between rounds.
- Added `DrainMainSell`: if MAIN reaches its sell threshold after a trade round, child stays held while MAIN sells, then MAIN returns to rendezvous and continues the same child session.
- Added one round-wide transfer-click budget shared by all `CHUYỂN ĐỒ` rows/group repeats so MAIN is not intentionally pushed below its sell threshold.
- Shared ACC CON workflow now requires at least one `CHUYỂN ĐỒ` row to prevent a click-only persistent drain loop.
- Preserved shared MAIN/global ACC CON model, DỒN ĐỒ OFF, BĐPT REAL INPUT, sell-sequence persistent FREEZE ALL and fixed CON1->CON6 priority.

## v0.2.7
- Replaced per-CON trade workflows with one global `CHUỖI GD ACC CON` shared by CON1..CON6.
- Runtime binds every CON-targeted row to the active transaction child selected by BĐPT; MAIN references remain shared.

## v0.2.6
- Added `DỒN ĐỒ: BẬT/TẮT` and independent auto-train/sell mode when OFF.
- Added sell-sequence cloning between accounts.
