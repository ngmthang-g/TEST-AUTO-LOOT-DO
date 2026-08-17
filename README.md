# ThanLong Item Consolidator v0.2.0

v0.2.0 replaces the v0.1 console prototype with the proven Win32 GUI/client-management/death-route foundation from the supplied ThanLong Clean Route v1.5.9 source.

## What is implemented

- Real Win32 GUI: the EXE opens even with zero game clients.
- Multi-client scan with character/RoleID, PID, state, Map/X/Y and authoritative `FreeBagSpace`.
- Per-account role persisted by RoleID: `MAIN`, `CON 1` ... `CON 6`, or off.
- Exactly one MAIN/CHILD trade transaction at a time.
- Round-robin selection of CHILD accounts when `FreeBagSpace <= child threshold`.
- MAIN blocks new trades when `FreeBagSpace < MAIN sell threshold` and gives existing Auto Sell priority.
- CHILD roles are hard-disabled from Auto Sell.
- Cooperative trade macro runner: death/map transition is still observed between macro steps; an unsafe pair aborts and releases the lock so donor death/route recovery can resume.
- Runtime UI clicks use background `PostMessage` mouse messages. No `SetCursorPos`, no `SendInput`, no foreground steal.
- Trade macro coordinates are normalized 0..1 and auto-scale per client window.
- `trade_give_items_child` is dynamically click-capped by MAIN remaining capacity.

## Donor behavior preserved

The supplied v1.5.9 bridge and controller state machine remain the source for identity, death/life, map, position, AutoFight and free bag state, including death-session cold restart, Underworld recovery, route ownership reacquire and return-to-train logic.

## First runtime test

1. Keep `ThanLongItemConsolidator_v0.2.0.exe` and `ThanLongCleanRouteBridge.dll` together.
2. Open two visible game clients.
3. Run the EXE and press `QUÉT CLIENT`.
4. Select one row and assign `MAIN`; select the other and assign `CON 1`.
5. Configure the same train spot and the existing revive/auto points as in Clean Route v1.5.9.
6. Edit the macros in `macros/` with the tested trade click coordinates/order. Remove `UNCONFIGURED` only from macros that are actually ready.
7. Press `NẠP MACRO`, tick both accounts, then `BẮT ĐẦU ACC TICK`.
8. First prove one MAIN + one CHILD. Add CON2..CON6 only after background click works reliably at two window sizes.

## Important runtime proof still required

Compile/CI success cannot prove Unity accepts `PostMessage` for every UI button. If a specific button ignores background messages, test and document that exact control before expanding the click backend; do not silently fall back to physical-mouse `SendInput`.
