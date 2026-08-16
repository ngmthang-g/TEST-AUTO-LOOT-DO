# PROJECT KNOWLEDGE — TESTauto-don-do-acc-chinh

## Project Identity
- Name: ThanLong Item Consolidator
- Repository: `ngmthang-g/TESTauto-don-do-acc-chinh`
- Branch under development: `agent/item-consolidator-v0.1`
- Version: `v0.1.0`
- Platform: Windows x64, native C++20/CMake
- Action policy: pure background window click only
- Runtime evidence: `UNTESTED`

## User Goal
Một tổ đội có 1 acc chính và 1..6 acc con. Tất cả train; acc con tự dồn đồ cho acc chính khi túi gần đầy; acc chính nhận đồ và tự bán khi túi gần đầy. Tool điều phối để không có hai acc con tranh giao dịch cùng lúc.

## Hard Requirements
1. `clinent-game-than-long-DATA-2222` là read-only knowledge source; không phát triển code ở đó.
2. Project hiện tại không dùng internal Game/Lua/packet action.
3. Không inject DLL/hook bridge.
4. Click nền không di chuyển con trỏ vật lý.
5. Tọa độ click phải scale theo kích thước cửa sổ.
6. Tối đa 6 acc con / 6 target slots.
7. Chỉ một trade transaction toàn hệ thống tại một thời điểm.
8. Acc con không auto-sell.
9. Sau mỗi trade rescan túi tất cả acc mặc định.
10. Delay/repeat/số click phải sửa ngoài source qua macro files.

## Knowledge-base facts used only for semantic understanding
The canonical research repo verifies that selected-player UI has stable RoleID/social action semantics and that Trade is a real selected-player action (`C_OtherRoleCommand.Trade = 7`, `CMD_OTHER_ROLE_COMMAND = 200051`). This project deliberately does **not** emit that packet; the fact is used only to validate that the visible “select player -> trade” flow is semantically legitimate and worth automating by click.

Canonical inventory knowledge also verifies that bag free-space exists semantically in the client, but this project does **not** call `Game.GetFreeBagSpace()`. v0.1.0 uses a visual bag-grid scanner to honor the pure-click/no-internal constraint.

## Current Implementation
- visible top-level window discovery by title substring;
- user selects MAIN and ordered CHILD list;
- ordered CHILD list maps to trade slot 1..6;
- normalized background click (`PostMessage` or `SendMessageTimeout`);
- per-click auto-resize;
- external macro DSL: sleep/click/grid;
- visual bag scan over configurable grid;
- mouse-assisted one-time bag geometry wizard;
- empty-slot one-time calibration;
- uncertain-scan fail-closed guard;
- separate child/main free-slot thresholds;
- dynamic transfer click cap based on MAIN free space + configurable max per trade;
- single global transaction mutex;
- round-robin CHILD selection;
- deterministic pure-click trade flow;
- MAIN-only sell flow;
- mandatory rescan after trade by default;
- optional calibrated visual death detector and recovery macro path.

## Key Hidden Assumptions / Risks
- `10 x 9 = 90` bag layout is currently only a default from the user's prior 90-click description; actual UI must be measured.
- Background `WM_MOUSE...` delivery may fail on this Unity build even while the window is visible. This is a runtime proof, not guaranteed by Win32 code compiling.
- Visual slot classification can drift with UI scale/theme/item icons; threshold must be tested and uncertain scans must not trigger action.
- Fixed macro delays can race network/UI latency. Initial values should be conservative.
- “Move to anchor” is represented as a macro because current project forbids internal movement calls.
- Exact trade popup/confirm count is not yet recorded from runtime video/screenshots in this repo, so shipped trade macros are intentionally placeholders.
- Item stacking means one item click does not guarantee one MAIN free slot disappears; therefore every transaction is followed by fresh scanning.

## v0.1.0 Evidence Status
### SOURCE
Implemented.

### CI
Pending GitHub Actions for this branch.

### RUNTIME
Untested.

## Next Runtime Test Order
1. Fill one harmless `click` macro and prove `post` mode clicks the correct UI without moving mouse.
2. Repeat on 2 differently sized game windows; verify normalized scaling.
3. If `post` misses, test `send` mode.
4. Run bag geometry wizard and calibrate one empty slot.
5. Compare scanner count against manual count on bags with many/medium/few empty slots.
6. Record exact trade click flow for one MAIN + one CHILD.
7. Fill macros and test one trade manually through the coordinator.
8. Test main stop threshold `<9` and MAIN-only sell macro.
9. Scale to 2..6 children and verify single-transaction queue.
10. Calibrate death detector and test recovery only after the death UI sample is available.

## Decisions
- DEC-001: keep v0.1.0 action layer pure background window click.
- DEC-002: remove old injection/bridge source from build rather than mixing architectures.
- DEC-003: use external macro files so click count/delay/repeat changes do not require rebuild.
- DEC-004: action input remains pure background window message; no fallback that steals physical mouse.
- DEC-005: bag scanner is fail-closed on uncertainty.
- DEC-006: do not invent live click coordinates; placeholders remain until runtime capture/test.
- DEC-007: cap item clicks per transaction from MAIN remaining free space; never blindly click an entire child bag into a nearly-full MAIN.
