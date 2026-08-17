# CHANGELOG

## v0.2.5
- MAIN sell click sequence now holds one persistent BĐPT `SEQUENCE LEASE`: FREEZE ALL begins when the click sequence starts and ends only when the sequence completes/aborts/stops.
- Fixed multi-select editor bug by editing the focused row rather than the first selected row.
- Fixed CON row transition from MAIN reference back to CON so description and CLICK/CHUYỂN ĐỒ controls become editable on the intended row.
- StopChecked now aborts an active trade when a participant is stopped.
- REC can reclaim stale click lease state, abort a hanging trade transaction, and release a persistent sequence lease before entering RECORDING.
