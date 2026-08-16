# Evidence Registry

## EVID-001 — Knowledge base navigation rule
The canonical client-data repository instructs future work to read `AI_INDEX.md`/knowledge base first and avoid broad reversing when verified/database facts already exist.

## EVID-002 — Selected-player trade is a real client semantic action
Canonical `analysis/16_PLAYER_INTERACTION_UI_API.md` verifies:
- `C_OtherRoleCommand.Trade = 7`;
- `CMD_OTHER_ROLE_COMMAND = 200051`;
- visible selected-player trade invitation is constructed from the target `RoleID`.

The current project does **not** emit this packet. The fact is used only to validate the UI flow being automated by click.

## EVID-003 — Inventory free-space has semantic truth in the client
Canonical knowledge verifies `Game.GetFreeBagSpace()` and bag item identity. The current project deliberately does not call it because the user constrained actions/state acquisition for this tool to pure UI/visual behavior. Therefore v0.1.0 implements a visual grid scanner and treats it as `RUNTIME UNVERIFIED` until calibrated.

## EVID-004 — Source implementation
Branch `agent/item-consolidator-v0.1` contains:
- normalized background click engine;
- macro DSL;
- visual bag scanner;
- MAIN/CHILD coordinator;
- single transaction guard;
- dynamic transfer cap;
- MAIN-only sell path;
- visual death recovery path.

Status: `SOURCE IMPLEMENTED`.

## EVID-005 — CI
Pending after first branch build. Do not mark PASS until GitHub Actions is green for the item-consolidator commit.

## EVID-006 — Runtime
No runtime test result has been provided yet for:
- background `WM_MOUSE...` acceptance;
- exact trade macro coordinates;
- bag geometry/thresholds;
- death signature;
- sell macro.

Status: `RUNTIME UNTESTED`.
