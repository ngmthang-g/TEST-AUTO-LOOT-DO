# CHANGELOG

## v0.1.0 — Item Consolidator foundation

### Project reset
- Repurposed repository from unrelated RemoteLoot PoC to the requested MAIN/CHILD item-consolidation tool.
- Removed internal bridge/injection architecture from the product design.

### Added
- Visible game-window discovery.
- Runtime MAIN selection + ordered 1..6 CHILD selection.
- CHILD order -> trade slot 1..6 mapping.
- Pure Win32 background click modes: `post` / `send`.
- Normalized coordinates auto-scaled to each current client size.
- External macro DSL with configurable delays, repeat counts and grid clicks.
- Mouse-assisted one-time bag geometry wizard.
- Visual 90-slot-default bag scanner with one-time empty-slot calibration.
- Uncertain visual scan guard.
- Separate CHILD trigger and MAIN stop/sell thresholds.
- Dynamic per-trade item-click cap derived from MAIN free space plus a configurable hard maximum.
- Single global trade/sell mutex.
- Round-robin waiting queue.
- Trade orchestration flow and MAIN-only sell flow.
- Rescan-all-after-trade behavior.
- Visual death-signature detector and `revive_return` recovery path (disabled until calibrated).

### Not runtime verified yet
- Whether the current Unity client accepts background `WM_MOUSE...` reliably.
- Exact live UI coordinates/macros.
- Bag-grid geometry and visual thresholds.
- Exact trade confirmation sequence.
- Sell path.
- Death visual signature and recovery macro.
