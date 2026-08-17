# VERSION v0.2.8 — Safe rendezvous, grouped loops, persistent child drain

## Requested behavior
1. Avoid risky field trading: after a FULL child is chosen, MAIN + that child stop auto and travel to one user-captured trade rendezvous before any trade click.
2. Allow 2/3/4/... consecutive trade rows to be grouped and repeat the entire mini-sequence before the outer workflow proceeds.
3. Child must be FULL to enter a drain session, but once selected it remains the active child until its own configured free-slot target is reached.

## Runtime contract
`FULL=0 -> pin MAIN+CONn -> rendezvous both -> shared trade workflow -> verify child bag -> repeat same child until target`.
If MAIN reaches its sell threshold after a round: `hold child -> MAIN sell -> MAIN returns rendezvous -> continue same child`.

## Safety
- Do not click trade unless both participants reach rendezvous and are stable.
- Abort on participant death/map instability/state loss.
- Global round transfer budget is shared by every transfer row and group repeat.
- All physical clicks still use BĐPT REAL INPUT.
