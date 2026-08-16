# Evidence Registry — Item Consolidator v0.1.0

## EVID-001 — Knowledge-base-first rule
Canonical project instruction: read `AI_INDEX.md` / knowledge base first; do not broad reverse-engineer the client unless an exact fact required for the task is missing from VERIFIED/database.

## EVID-002 — Selected-player Trade is a real client action
Canonical `analysis/16_PLAYER_INTERACTION_UI_API.md` verifies:
- `C_OtherRoleCommand.Trade = 7`;
- `CMD_OTHER_ROLE_COMMAND = 200051`;
- the shipped selected-player UI constructs the trade request from target `RoleID`.

This tool does **not** emit that packet. The evidence is used only to understand the visible target-selection/trade UI that the click macro must reproduce.

## EVID-003 — Bag free-space has semantic truth, but this project stays visual
Canonical client research verifies bag/free-space semantics exist. The current user constraint forbids using internal Game/Lua/packet action/state routes for this tool, therefore v0.1.0 implements a visual slot scanner and treats its accuracy as runtime evidence, not as a guaranteed fact.

## EVID-004 — Source implementation
Branch `agent/item-consolidator-v0.1` implements:
- visible multi-window discovery;
- MAIN + ordered 1..6 CHILD assignment;
- normalized background `WM_MOUSE...` click engine (`post`/`send`);
- external macro DSL (`sleep`, `click`, `grid`);
- visual bag scanner + calibration + uncertainty fail-closed guard;
- separate MAIN/CHILD thresholds;
- dynamic per-trade item-click cap from MAIN free space;
- global transaction mutex + round-robin child queue;
- MAIN-only sell flow;
- rescan after trade;
- optional visual death/revive recovery path.

Status: `SOURCE IMPLEMENTED`.

## EVID-005 — Windows x64 CI
Final source build after Win32 macro compatibility fixes:
- commit: `2a2b7ea4f2c2ef1e2511bb820a2754e47be19fab`;
- workflow run: `31961357854` / run #17;
- Configure x64: PASS;
- Build Release: PASS;
- Stage artifact: PASS;
- Upload artifact: PASS;
- artifact: `ThanLongItemConsolidator-v0.1.0-win-x64`;
- artifact ID: `9267326722`;
- artifact digest: `sha256:b6ec29a3b9d931a5ae9add0442bc9b6ff9c04b94c91f81b79d034892132f4e77`.

Status: `CI PASS`.

## EVID-006 — Runtime remains unverified
No runtime evidence has yet proved:
- this Unity client accepts `PostMessage` or `SendMessageTimeout` background clicks reliably;
- exact normalized coordinates for train/anchor/trade/sell/revive UI;
- actual bag grid geometry and visual thresholds;
- exact trade confirmation timing;
- death-screen visual signature and recovery sequence.

Status: `RUNTIME UNTESTED`.

## EVID-007 — Placeholder safety
Shipped macro files intentionally contain `UNCONFIGURED`, an invalid macro command. They therefore fail parsing until replaced with tested click steps. This prevents an empty placeholder from being treated as a successful trade/sell/revive action.
