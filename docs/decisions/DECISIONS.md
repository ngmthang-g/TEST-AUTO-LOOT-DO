# Decisions — Item Consolidator

## DEC-001 — Action layer is pure background click
No DLL injection, IL2CPP bridge, Lua/Game API invocation or packet sending. Runtime actions are Win32 window-message clicks only.

## DEC-002 — Normalized coordinates
All macro coordinates are stored in `[0,1]` and scaled from the current client rectangle at each click. No fixed resolution assumption.

## DEC-003 — One global transaction
Only one trade/sell transaction may own MAIN at a time. `transactionMutex` is the hard guard.

## DEC-004 — MAIN owns target selection
Trade invitation macro always runs on MAIN. CHILD only accepts, gives items and confirms.

## DEC-005 — Visual bag state, fail closed
Bag free-space decisions come from visual slot classification because the current project explicitly forbids internal API calls. An uncertain scan cannot trigger trade/sell.

## DEC-006 — Separate thresholds
- CHILD eligible when `freeSlots <= child_trigger_free_slots`.
- MAIN sells when `freeSlots < main_stop_free_slots`.

## DEC-007 — Dynamic transfer cap
The number of grid item-clicks during `trade_give_items_child` is capped by both:
- `max_transfer_clicks_per_trade`;
- MAIN remaining free capacity relative to the stop threshold.

This prevents blindly stuffing a large child bag into a nearly full MAIN.

## DEC-008 — No guessed live coordinates
Repository macro files ship as placeholders. Runtime coordinates must be recorded/tested on the actual client before unattended use.

## DEC-009 — Visual death detector is calibration-gated
The death detector exists but is disabled until a stable death-screen patch is captured. Recovery is macro-based: `revive_return -> move_anchor -> start_train -> rescan`.
