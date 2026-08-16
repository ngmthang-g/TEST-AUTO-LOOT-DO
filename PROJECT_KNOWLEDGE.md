# PROJECT KNOWLEDGE — TESTauto-don-do-acc-chinh

## Project Identity
- Name: ThanLong Item Consolidator
- Repository: `ngmthang-g/TESTauto-don-do-acc-chinh`
- Development branch: `agent/item-consolidator-v0.1`
- Version: `v0.1.0`
- Platform: Windows x64, native C++20/CMake
- Action policy: pure background window click only
- Source/CI: `PASS`
- Runtime game evidence: `UNTESTED`

## User Goal
One MAIN account and 1..6 CHILD accounts train together. CHILD accounts transfer items to MAIN when their bags approach full. MAIN receives items and sells when its own bag approaches full. The tool coordinates windows so only one CHILD can trade at a time.

## Hard Requirements
1. `clinent-game-than-long-DATA-2222` is read-only research/knowledge; code changes belong only in this repo.
2. No internal Game/Lua/packet action path.
3. No DLL injection/hook bridge.
4. Runtime click must not move or occupy the physical mouse.
5. Every click coordinate auto-scales to the current game-client rectangle.
6. 1..6 CHILD accounts map to ordered trade slots 1..6.
7. One global trade/sell transaction owner at a time.
8. CHILD never auto-sells.
9. Rescan all bags after each trade by default.
10. Click count, delay and repeat are external macro data, not hard-coded behavior.
11. Unconfigured macros must fail closed instead of pretending success.

## Canonical facts used only for semantic understanding
Canonical research verifies selected-player Trade exists and is driven by target RoleID. This project intentionally does not emit that internal request; the evidence is only used to validate the visible UI flow that click macros reproduce.

Canonical research also verifies bag free-space exists semantically, but v0.1.0 does not call it. Bag state for orchestration is visual-only to honor the no-internal constraint.

## Current Implementation
- visible top-level game-window discovery by title substring;
- MAIN selection + ordered 1..6 CHILD selection;
- CHILD order maps to `trade_invite_1..6`;
- background click via `PostMessage` or `SendMessageTimeout`;
- normalized 0..1 coordinate scaling on every click;
- external macro DSL: `sleep`, `click`, `grid`;
- fail-closed `UNCONFIGURED` shipped macros;
- visual bag-grid scanner with geometry wizard and empty-slot calibration;
- uncertain visual scan prevents trade/sell decisions;
- CHILD trigger default `freeSlots <= 9`;
- MAIN sell default `freeSlots < 9`;
- dynamic transfer grid-click cap from MAIN remaining capacity plus configurable hard cap;
- round-robin CHILD selection;
- global transaction mutex;
- flow: stop train -> both move anchor -> MAIN invite -> CHILD accept/give/confirm -> MAIN confirm -> restart -> rescan;
- MAIN-only sell -> rescan -> move anchor -> train;
- optional visual death signature -> revive_return -> anchor -> train -> rescan.

## Important Risks / Missing Runtime Data
- A compiling `WM_MOUSE...` click path does not prove this Unity/InputSystem accepts it. `post` and `send` require live proof.
- `PrintWindow` can return a frame that is stale/black on some Unity render paths; bag/death capture must be compared with visible UI.
- 10x9=90 slots is only a default assumption until measured on the live bag UI.
- Visual slot thresholds may drift with UI scale, icon animation/theme or capture behavior; uncertain scans are intentionally fail-closed.
- Fixed timing can race network/UI latency; initial macro delays should be conservative.
- Exact trade/sell/revive coordinates are deliberately not guessed in the repository.
- Death detection is checked by the single-threaded coordinator between actions. v0.1.0 does **not** preempt a macro that is already mid-sequence; recovery begins on the next coordinator cycle.
- Item stacking means one CHILD item click is not guaranteed to consume one MAIN free slot; fresh rescan after each trade is authoritative for the next decision.

## CI Evidence
Commit `2a2b7ea4f2c2ef1e2511bb820a2754e47be19fab`, GitHub Actions run `31961357854` (#17): configure/build/stage/upload all PASS. Artifact `ThanLongItemConsolidator-v0.1.0-win-x64`, ID `9267326722`.

## Runtime Test Order
1. Replace `UNCONFIGURED` in one safe click flow with measured normalized coordinates.
2. Prove `post` clicks the correct visible Unity UI without moving the real cursor.
3. Repeat the same macro on two different window sizes to validate normalized scaling.
4. If `post` misses, test `send`; do not fall back to `SendInput`/`SetCursorPos` because that violates the user requirement.
5. Calibrate bag geometry + one known empty slot and compare scan counts to manual counts at multiple fill levels.
6. Record/fill exact one-MAIN + one-CHILD trade macros and test one transaction.
7. Verify MAIN stops receiving below 9 free slots and runs MAIN-only sell.
8. Scale to 2..6 CHILD and confirm transaction serialization/round-robin behavior.
9. Calibrate a stable death UI patch and test revive/return only after trade/bag flow is stable.

## Decisions
- Keep action layer pure background click.
- Remove old RemoteLoot bridge/injection source rather than mix architectures.
- Externalize mutable UI sequencing as macros.
- Never use a physical-mouse fallback silently.
- Fail closed on uncertain bag scans and unconfigured macros.
- Do not invent live coordinates.
- Cap transfer clicks using MAIN capacity and always rescan after trade.
