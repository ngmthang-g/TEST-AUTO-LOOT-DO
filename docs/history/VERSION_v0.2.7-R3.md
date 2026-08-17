# VERSION v0.2.7-R3

## Scope
- Based directly on v0.2.7-R2 final controller SHA256 `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- User explicitly ordered: **change only deletion of selected rows inside the trade click-sequence editor; do not touch anything else**.

## User Request
The trade click-sequence editor already supports Ctrl/Shift multi-selection, but `- XÓA` deleted only one focused row. Allow deleting all selected coordinate/click rows in one action.

## Implementation
Only `DeleteTradeSequenceRow()` changes in runtime source:
- Read all selected rows using the existing `SelectedRows(tradeSeqList_)` helper.
- Validate and sort the selected indices.
- Delete rows in reverse order so vector indices do not shift underneath the operation.
- In the shared MAIN editor only, repair ACC CON `MAIN #n` references across multiple deleted MAIN rows: deleted refs become invalid (`-1`), surviving refs shift by the number of earlier deleted MAIN rows.
- In the ACC CON editor, run the already-existing `NormalizeTradeGroups()` after deletion.
- Single-row deletion remains supported by the same path.

## Protected / Not Changed
- F4 pause/hotkey.
- DỒN ĐỒ ON/OFF.
- Trade rendezvous and both-arrived wait.
- Group repeat execution and grouping UI.
- AutoFight fallback movement.
- Five-click model.
- BĐPT/REAL INPUT and exactly two raw `SendInput(` call sites.
- Sell/REC/train/route/revive logic.
- No old v0.2.8/v0.2.9 code imported.

## Source Identity
- R2 controller SHA256: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`
- R3 controller SHA256: `a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf`
- R3 patch SHA256: `3b013821934c882cce8dc755894f66ab835feec394d3433015127a8792fc2136`

## Build / Runtime
- Local patch/apply and reference-remap tests: PASS.
- CI #60 / run `32048547405`: failed before compile on patch transport EOL checksum only; no runtime-source change was made.
- CI #61 / run `32048648947`: **CI PASS / BUILD PASS** — rehydrate, Windows x64 build, route/rotation/trade tests, static R3 audit and artifact upload all passed.
- Runtime: **RUNTIME UNTESTED** until user tests multi-delete in the live tool.
