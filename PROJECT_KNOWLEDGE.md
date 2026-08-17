# PROJECT KNOWLEDGE — ThanLong Item Consolidator

## Current development version
v0.2.2 — REAL INPUT + Central Coordinator. Donor behavior remains Clean Route v1.5.9 for death/revive/route/train recovery.

## User-confirmed runtime finding
Background `PostMessage` click from v0.2.1 does not operate reliably in the live game. v0.2.2 intentionally restores physical mouse input from donor 1.5.9: foreground game window + SetCursorPos + SendInput.

## Architecture invariant
There is one master tool and one central coordinator. Loaded MAIN/CON windows are branches/resources of that coordinator, not independent automations competing for the physical mouse.

Only one account/action path may own REAL INPUT at a time. A trade transaction takes the global action/mouse lock from MAIN preparation until the final configured trade click completes.

## Account roles
- OFF
- MAIN
- CON1..CON6

Roles persist by RoleID.

Each CON profile also stores `TradeSelectX/Y/W/H`: the point MAIN must click to select that specific CON before the trade sequence begins. The point is captured against MAIN's client rectangle but belongs logically to the CON profile.

## Authoritative bag rules
- MAIN `FreeBagSpace <= 6` => sell priority; do not start a new CON trade.
- CON eligible only when `FreeBagSpace == 0`.
- Multiple FULL CONs => fixed priority CON1 -> CON2 -> ... -> CON6.
- No round robin.

## Preparation flow
1. Coordinator chooses the highest-priority FULL CON.
2. Global transaction/mouse ownership is acquired.
3. MAIN uses donor 1.5.9 train recovery to stop AutoFight and return to its selected train target.
4. Only after MAIN is standing ready does selected CON do the same.
5. Both must be alive/stable/at train target with route/fight stopped.
6. MAIN performs REAL INPUT click at selected CON's saved `tradeSelectPoint`.
7. Trade click sequence begins.

## Trade click sequence editor
The old `NẠP MACRO` button merely reloaded manually edited `.macro` files. It is not the primary v0.2.2 workflow.

`CHUỖI CLICK GD` stores an ordered vector of steps in INI section `[TradeSequence]`.

Each step contains:
- target: MAIN or selected CON;
- kind: normal CLICK or CHUYỂN ĐỒ;
- description;
- scaled click point X/Y/baseW/baseH;
- delay after click;
- repeat count.

Editor actions: add, delete, move up/down, save row, capture F8, test row.

CHUYỂN ĐỒ is forced to CON and dynamically capped so MAIN is not intentionally filled below its sell reserve.

## REAL INPUT contract
Runtime click path intentionally uses:
- SetForegroundWindow
- BringWindowToTop
- SetCursorPos
- SendInput LEFTDOWN/LEFTUP

If the intended game window cannot become foreground, fail closed rather than clicking the wrong window.

The cursor is intentionally occupied during actions. Other tool actions must not execute concurrently.

## Central status UI
The GUI prominently exposes `BỘ ĐIỀU PHỐI TRUNG TÂM` and a master status line. Expected messages describe ownership and phase, e.g. MAIN prep, CON2 prep, mouse -> MAIN, MAIN <-> CON2 step N/M.

Per-account rows remain for identity, role, bag/map/position and branch status.

## Legacy compatibility
`MacroLibrary`, BackgroundClicker and `macros/` may remain in source/package for historical compatibility, but active v0.2.2 trade orchestration uses the visual TradeSequence + REAL INPUT path. Do not reintroduce `trade_invite_N` as an active dependency without an explicit design decision.

## Build integrity
CI rehydrates the exact v1.5.9 donor, applies the verified v0.2.1 patch/compile fix, verifies the known v0.2.1 controller hash, then assembles and applies the v0.2.2 patch. Final controller hash must match the recorded v0.2.2 SHA before compilation.

## Runtime proof still required
A green CI build does not prove live game click geometry/timing. First live proof should use 1 MAIN + 1 CON: capture selector point, build a minimal trade sequence, test individual rows, then test one fully coordinated transaction before scaling to more CONs.
