# VERSION v0.2.7 — Shared MAIN / ACC CON trade sequences

## User-facing behavior
1. Trade setup is reduced to exactly two reusable definitions: `CHUỖI GD MAIN` and `CHUỖI GD ACC CON`.
2. `CHUỖI GD ACC CON` is global. CON1..CON6 no longer own separate workflow copies.
3. When BĐPT starts a transaction with CONn, every child-targeted row in the shared workflow executes on that exact CONn window.
4. MAIN reference rows still execute on MAIN, preserving an ordered workflow that may interleave MAIN and CON actions.
5. `CHUYỂN ĐỒ` is always executed on the active CON.
6. Opening the shared CON editor from any CON makes that CON the donor for REC/LẤY TỌA/TEST only; the saved sequence itself is global.
7. Old v0.2.6 per-CON sequences are migrated once. The lowest existing CON slot wins deterministically; legacy copies are retained but no longer active.
8. The child trade-sequence button no longer overlaps the sell-sequence button.

## Unchanged behavior
DỒN ĐỒ BẬT/TẮT, MAIN sell priority, FULL-only CON trigger, CON1->CON6 priority, per-CON MAIN-selection coordinate, central BĐPT input arbitration, REC safety, sell-sequence persistent FREEZE ALL, independent auto-train mode and Clean Route v1.5.9 donor logic remain unchanged.
