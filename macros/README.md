# Trade macro configuration v0.2.0

The GUI always opens even when these macros are not configured. The coordinator simply shows `macro ... chưa cấu hình` and does not start a trade.

Required common macros: `stop_train`, `move_anchor`, `trade_accept_child`, `trade_give_items_child`, `trade_confirm_child`, `trade_confirm_main`, `start_train`.
Required MAIN invite macro for each configured child slot: `trade_invite_1` ... `trade_invite_6`.

DSL:
- `sleep 800`
- `click 0.5000 0.7500 1 80 300`
- `grid 0.4200 0.3300 5 6 0.0550 0.0600 9 100 300`

`trade_give_items_child` receives a dynamic maximum click count based on current MAIN free slots and the configured MAIN sell threshold.
