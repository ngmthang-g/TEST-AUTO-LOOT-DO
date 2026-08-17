# v0.2.0 migration — Clean Route v1.5.9 donor

## Goal
Replace the v0.1 console-first prototype entrypoint with the proven Win32 GUI/client-management foundation from `ThanLongCleanRoute v1.5.9`, while keeping the item-consolidation scheduler and background-click constraint.

## Donor facts verified from supplied v1.5.9 source

- GUI entrypoint is `wWinMain` and creates a real Win32 window with `CreateWindowExW`.
- Multi-client list already exposes character/RoleID, PID, state, Map/X/Y and `freeBagSpace`.
- Bridge/read-only snapshot already exposes death/life state, map, position, AutoFight and `GetFreeBagSpace()`.
- Death session cold restart, Underworld recovery, route-ownership reacquire and return-to-train flow already exist and should be preserved as donor behavior.
- Current donor click path `RealInputClick()` uses `SetForegroundWindow` / `SetCursorPos` / `SendInput`; this is NOT acceptable for item-trade runtime because it captures physical mouse/focus.

## v0.2 architecture decision

### Keep from v1.5.9
- Win32 GUI and client discovery/list.
- RoleID-based per-account persistence.
- Read-only bridge snapshot for death, map, position, AutoFight and free bag slots.
- Proven death -> revive -> Underworld -> route -> train recovery flow.
- Existing route/train-spot state machine and its cold-restart guards.

### Keep from v0.1 item consolidator
- MAIN / CHILD model (1 MAIN, 1..6 CHILD).
- One global trade transaction at a time.
- Round-robin CHILD scheduling.
- Separate CHILD trigger and MAIN stop thresholds.
- Full rescan after each trade.
- Macro repeat/delay model.
- Window-relative coordinate scaling.

### Remove from v0.1 runtime
- Visual bag scanner.
- Visual death signature detector.
- Console-only account selection as the primary UI.

### Trade actions
Trade UI is intentionally treated as a user-recorded click macro. No internal Game/Lua/packet action is required for invite/accept/add-item/confirm.

Runtime trade clicks must use a background window-message backend (`PostMessage` first, optional `SendMessageTimeout` fallback). They must NOT use `SendInput`, `SetCursorPos` or foreground-stealing code.

The v1.5.9 `RealInputClick()` path may remain for legacy Clean Route behavior during migration, but the new trade macro engine must never call it.

## Coordinator rules

1. CHILD becomes eligible when its reliable `freeBagSpace <= child_trigger_free_slots`.
2. Only one CHILD may own the trade lock with MAIN.
3. Other CHILD accounts continue their normal train/recovery state machines while waiting.
4. MAIN and chosen CHILD stop train and converge on the configured common train anchor before trade macro starts.
5. MAIN executes `trade_invite_<child-slot>`; CHILD executes accept/give/confirm; MAIN confirms.
6. After the transaction, rescan every selected account.
7. If MAIN `freeBagSpace < main_stop_free_slots`, do not start another CHILD trade; MAIN enters sell/recovery flow first.
8. Never infer trade success from click count alone; authoritative free-bag snapshot after the sequence decides the next state.

## GUI target

The client list should add an item-consolidation role/status layer:

- Role: MAIN / CON1..CON6 / OFF
- Free bag slots
- Training / waiting / trading / selling / recovering
- Current trade owner
- Queued CHILD order

The EXE must always open the GUI even if no game client is available. Missing clients/config/macros must be shown in the GUI/log instead of immediately terminating like v0.1 console prototype.

## Implementation order

1. Transplant v1.5.9 GUI + bridge/core as v0.2 executable foundation.
2. Confirm GUI opens and scans multiple clients.
3. Confirm death/revive/return-to-train behavior is unchanged.
4. Confirm free-bag values are stable across accounts.
5. Add MAIN/CON1..6 role assignment and queue view.
6. Add isolated background-click macro engine.
7. Add trade coordinator using user-supplied click coordinates/order.
8. Runtime proof with one MAIN + one CHILD before enabling 2..6 CHILD accounts.

## Safety/fail-closed conditions

Do not start a trade if any required snapshot is stale/unavailable, either selected window is missing/minimized/unresponsive, a death/recovery phase is active, or MAIN capacity is below the configured threshold.
