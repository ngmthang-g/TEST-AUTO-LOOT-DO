# CHANGELOG

## v0.2.3
- Promoted BỘ ĐIỀU PHỐI TRUNG TÂM from transaction status/lock to mandatory REAL INPUT arbiter.
- Added per-click lease: request -> FREEZE ALL -> one target click -> RESULT -> UNFREEZE ALL.
- Removed v0.2.2 whole-transaction global freeze; workflow participants use HOLD while unrelated accounts may run between click leases.
- Hid coordinate/sequence tables from the main UI and added role-specific buttons.
- MAIN now has `CHUỖI CLICK BÁN ĐỒ` and one shared `CHUỖI GD MAIN`.
- Each CON has its own `CHUỖI GD CONx`; rows can execute on that CON or reference `MAIN #n` shared actions.
- Preserved per-CON selector coordinate on MAIN.
- Preserved MAIN <=6 sell priority, FULL-only CON trigger, fixed CON1..CON6 priority, and Clean Route v1.5.9 route/death foundation.
