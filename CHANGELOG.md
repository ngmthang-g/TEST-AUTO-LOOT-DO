# CHANGELOG

## v0.2.1
- MAIN sell trigger changed to <= 6 free slots (configurable threshold default 6), with absolute priority over trade.
- CON trade trigger changed to exact FULL only.
- Removed round-robin; fixed priority CON1 -> CON6.
- Removed trade move_anchor/start_train dependency. MAIN then CON use donor v1.5.9 train recovery/route logic.
- Added global atomic trade freeze: other death/revive/map-confirm/route/sell/fight actions wait until MAIN final confirm completes.
- After trade, donor core resumes and handles pending state before fighting.
- `trade_invite_N` is the child-specific MAIN target/select + trade-open macro.
