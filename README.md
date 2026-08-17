# ThanLong Item Consolidator v0.2.1

Core donor: ThanLong Clean Route v1.5.9.

Exact coordinator rules:
- MAIN and every CON use the original v1.5.9 death/revive/route-to-train state machine.
- MAIN FreeBagSpace <= 6: Auto Sell has absolute priority; no trade may start. Threshold remains editable but defaults to 6.
- CON eligible only when FreeBagSpace == 0 (FULL).
- If multiple CON are FULL, fixed priority is CON1 -> CON2 -> ... -> CON6; no round-robin.
- Preparation order: hold MAIN, stop AutoFight + route MAIN to its selected train target; then do the same for selected CON.
- Only after both are alive, at their selected train target, dismounted, AutoPath OFF and AutoFight OFF does the atomic trade chain start.
- `trade_invite_N.macro` is MAIN-side child-N target selection/trade-open macro. Configure its first click as the custom coordinate used by MAIN to select that child.
- During Invite/Accept/Give/ConfirmChild/ConfirmMain the whole automation is frozen. Read-only state still refreshes, but death/revive/map-confirm/route/sell/fight cannot preempt.
- After ConfirmMain completes, the freeze is released. Then the normal v1.5.9 priority handles any pending death/revive/map confirmation/route state before fighting resumes.
- Runtime clicks remain background PostMessage; no physical cursor capture.

Trade macro files are under `macros/`. They stay fail-closed while containing `UNCONFIGURED`.
