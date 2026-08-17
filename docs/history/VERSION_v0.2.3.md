# VERSION v0.2.3 — Central Arbiter + Role-specific hidden editors

## User request captured
The main GUI must stay compact. Sell/trade coordinate sequences are hidden behind role-specific buttons. The central coordinator must genuinely control, block, stop, resume and serialize account actions. Every physical click request must be approved by BĐPT first.

## Implemented behavior
- MAIN role: button `CHUỖI CLICK BÁN ĐỒ`; button `CHUỖI GD MAIN`.
- CON role: button `CHUỖI GD CONx`; per-CON selector coordinate.
- Shared MAIN sequence is defined once and reusable by all CON plans.
- A CON plan can alternate executors, e.g. CON click -> MAIN #2 -> CON click -> MAIN #4.
- BĐPT resolves the target account/window for every row.
- For each REAL INPUT click BĐPT marks global input freeze, foregrounds only the granted target, performs the click, receives success/failure, then unfreezes.
- MAIN + active CON stay workflow-held while preparing/trading so their normal state machines cannot interfere.
- Other accounts are not frozen for the entire transaction; they can progress between physical click leases and must themselves request BĐPT when they need the mouse.

## Preserved logic
MAIN free slots <=6 => sell priority. CON must be FULL. Multiple FULL CONs use fixed CON1..CON6 priority. MAIN prepares first, then selected CON, using donor 1.5.9 route/recovery.

## Source integrity
v0.2.3 is a checksum-verified patch on top of the exact v0.2.2 controller. Final expected controller SHA256: `4f7069a0ae47b417a2a4ccf8da4bfd3d4019ae216d88e01070e51c7e0e085fe4`.
