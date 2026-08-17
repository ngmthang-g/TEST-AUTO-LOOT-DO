# CHANGELOG

## v0.2.7-R3 — 2026-08-17

### Requested
- Only change the trade click-sequence editor so `- XÓA` deletes all currently selected rows instead of only one focused row.
- Keep every other runtime feature/code path unchanged.

### Changed
- `DeleteTradeSequenceRow()` now uses the editor's existing multi-selection (`SelectedRows`).
- Selected rows are deleted from highest index to lowest index.
- When deleting multiple rows from `CHUỖI GD MAIN`, shared ACC CON `MAIN #n` references are repaired across all removed MAIN rows.
- Single-row delete remains supported by the same code path.

### Protected / Unchanged
- F4, DỒN ĐỒ, TỌA GD rendezvous, group repeat, AutoFight fallback, five-click model, BĐPT/REAL INPUT, sell/REC/train/route/revive logic.
- No v0.2.8/v0.2.9 code imported.

### Source
- R2 base controller: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- R3 controller: `a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf`.
- R3 patch: `3b013821934c882cce8dc755894f66ab835feec394d3433015127a8792fc2136`.
- Runtime status: **RUNTIME UNTESTED** until live multi-delete is tested.

## v0.2.7-R2 — 2026-08-17

### Requested
- Verify `DỒN ĐỒ: BẬT/TẮT` actually gates consolidation.
- FULL CON immediately holds MAIN+CON and sends both to one user-captured TỌA GD.
- First arrival waits; both must be confirmed at TỌA GD before existing trade clicks.
- Add grouped mini-sequence repetition for one or more consecutive trade rows.
- Remove all per-CON selector coordinates.
- AutoFight-stop failure must not deadlock movement; retry after map transition.
- Merge old AUTO + DỪNG AUTO 1 into one `AUTO` point.
- Preserve F4 exactly.

### Added / Changed / Fixed
- Added global TỌA GD capture/persistence and dedicated `tradeTravel*` rendezvous state.
- Both transaction participants are `tradeHeld` immediately on FULL CON selection.
- Added first-arrival hold/StopPath behavior and both-arrived readiness gate.
- Added group metadata/editor/runtime loop while preserving row repeat.
- Removed `tradeSelectPoint`, `TradeSelect*` persistence/UI/runtime.
- Per-account click array changed 6→5; `AUTO` replaces old duplicated StopAuto1 point.
- Train/sell/M87/trade movement can continue after bounded AutoFight-stop retries and retry after map change.
- `ReleaseTradeHolds()` now cleans all rendezvous holds/state.
- Restored untouched clean-v0.2.7 `PeriodicConfirmBusy()` and `HandleFightClicks()` after CI detected they had been accidentally deleted during the first edit pass.

### Build / CI
- Local clean-base patch chain: PASS, final controller `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- CI #55: FAILED before build due Windows EOL checksum mismatch; corrected by LF normalization.
- CI #56: rehydrate/configure PASS, BUILD FAILED due accidental removal of two untouched clean helpers; corrected by byte-exact restoration.
- CI #57 / run `32043612053`, job `95427111307`: **CI PASS**, including Windows x64 Build Release + route/rotation/trade tests + static requested-only audit + artifact upload.
- Runtime remains **RUNTIME UNTESTED**; CI does not upgrade it.

### Runtime
- Status: **RUNTIME UNTESTED**.
- Protected evidence: F4 behavior from clean v0.2.7 is user-confirmed useful; exact pause block is byte-protected.

### Next Version Notes
- First collect live evidence for rendezvous/map transitions/group repeat/DỒN OFF/F4 before redesigning any state machine.

## v0.2.7
- Replaced per-CON trade workflows with one global `CHUỖI GD ACC CON` shared by CON1..CON6.
- Runtime binds CON-targeted rows to active transaction child and retains shared MAIN references.
- DỒN ĐỒ ON/OFF, BĐPT, REC, sell FREEZE and donor core were preserved.

## v0.2.6
- Added `DỒN ĐỒ: BẬT/TẮT` and independent auto-train/sell while OFF.
- Added whole sell-sequence cloning.
