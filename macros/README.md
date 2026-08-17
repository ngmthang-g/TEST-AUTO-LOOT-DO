# Trade macros v0.2.1

All macro coordinates are normalized `0..1` and executed with background PostMessage.

`trade_invite_1.macro` ... `trade_invite_6.macro` are MAIN-side, child-specific macros. The first click should be the custom coordinate that selects that CON; subsequent clicks may open the trade UI. Add/delete as many `click` / `sleep` steps as needed.

Then the fixed target-window phases are:
- `trade_accept_child.macro`
- `trade_give_items_child.macro`
- `trade_confirm_child.macro`
- `trade_confirm_main.macro`

`stop_train`, `move_anchor`, and `start_train` are no longer used by the trade coordinator. Stopping AutoFight and returning to the train coordinate are performed by the donor v1.5.9 route/recovery logic.

Keep `UNCONFIGURED` in every macro that has not been tested. A required macro containing it will block the trade safely.
