# CHANGELOG

## v0.2.2 — REAL INPUT + Central Coordinator

### Changed
- Reverted runtime click path from background PostMessage to donor-style foreground physical mouse input: SetForegroundWindow + SetCursorPos + SendInput.
- Made the coordinator the single owner of the physical mouse/action channel; trade lock now begins before MAIN preparation.
- Promoted master coordinator state to a visible top-level GUI status line.
- Replaced the normal `NẠP MACRO` workflow with `CHUỖI CLICK GD` visual editor.

### Added
- Per-CON MAIN-side selector coordinate persisted by RoleID.
- F8 capture and TEST button for each CON selector coordinate.
- Visual ordered trade-step editor with MAIN/CON target, CLICK/CHUYỂN ĐỒ type, description, delay, repeat, F8 capture, row test, add/delete/up/down/save.
- Dynamic CHUYỂN ĐỒ repeat cap based on MAIN safe remaining capacity.
- Explicit coordinator phase/status messages showing which account owns the mouse and which trade step is running.

### Preserved from v0.2.1
- Exact Clean Route v1.5.9 death/revive/route foundation.
- MAIN `FreeBagSpace <= 6` sell priority.
- CON eligible only at `FreeBagSpace == 0`.
- Fixed CON1 -> CON6 priority; no round robin.
- MAIN prepares/returns to train target before selected CON.

### Compatibility
- Legacy macros directory remains packaged for history/backward compatibility, but active v0.2.2 trade flow no longer depends on `trade_invite_*.macro` files.

### Runtime status
- Source/build validation is automated in CI.
- Live game coordinate/timing and full one-MAIN/one-CON transaction remain runtime test items.

## v0.2.1 — Exact Trade Logic
- MAIN <=6 sell priority.
- CON FULL-only eligibility.
- Fixed CON1..CON6 priority.
- Exact v1.5.9 train recovery preparation.
- Atomic trade freeze after preparation.
