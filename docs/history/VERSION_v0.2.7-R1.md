# VERSION v0.2.7-R1

## Lineage
- Exact base branch/head: `agent/item-consolidator-v0.2.7-shared-child-trade-sequence` / `1308b28bd38fb044b9fceed3671820e45fb2cd23`
- Exact base controller SHA256: `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`
- v0.2.8 and v0.2.9 are excluded; no source or logic from them is used.

## Requested changes only
1. Multi-row grouping/repeat inside global `CHUỖI GD ACC CON`.
2. Per-CON free-slot target; FULL=0 is entry-only, selected CON stays pinned across rounds.
3. User-captured global `TỌA ĐỘ GIAO DỊCH`; MAIN+CON cancel existing map/train AutoPath and AutoFight before both route there, while trade HOLD blocks normal map/train until session ends.
4. Session ends when MAIN free slots <= existing sell threshold (default 6) OR child free slots >= configured target; then untouched v0.2.7 core resumes.
5. Auto-sell-full UI is a BẬT/TẮT pushbutton instead of a checkbox, with underlying sell logic unchanged.

## Integrity
- R1 controller SHA256: `575c289a7587d8de62d91124cf2d7601816d6bf1ff028e6941ecabaf3ae8d2d4`
- R1 patch SHA256: `5184f3c15d664143ce8a04c128cc1febfd443cdf13657ad6aea200e23e397520`
- R1 patch archive SHA256: `302426edc5bb31e260516d8b7a7596edd2dcbcd05dbec79418753b1862f08d9e`
- Exactly two `SendInput(` call sites remain.
