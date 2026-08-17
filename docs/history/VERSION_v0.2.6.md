# VERSION v0.2.6 — Consolidation toggle + sell sequence clone

## User-facing behavior
1. Top-level button shows `DỒN ĐỒ: BẬT` or `DỒN ĐỒ: TẮT`.
2. ON preserves the established consolidation scheduler.
3. OFF makes each running account independent: full bag (0 free slots) -> stop fight -> route to sell NPC -> run its own sell sequence -> return train -> resume AutoFight.
4. OFF never starts MAIN↔CON trade. If toggled off mid-trade, the transaction is aborted and both holds are released.
5. Sell macro sequence freeze behavior is unchanged: once the click sequence itself starts, BĐPT FREEZE ALL remains continuously active until sequence completion/abort.
6. Sell editor/REC work for MAIN, CON or unassigned selected accounts.
7. `LẤY CHUỖI CỦA ACC...` lists other scanned accounts with a non-empty sell sequence and replaces the current account's sequence after confirmation.

## Compatibility
The persisted INI key remains `Global/TradeEnabled` so v0.2.5 settings migrate without a format change.
