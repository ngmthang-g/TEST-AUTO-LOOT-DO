# PROJECT KNOWLEDGE — Item Consolidator v0.2.1

## Authoritative rules
MAIN sell threshold default = 6 and triggers at `freeBagSpace <= threshold`. Child trade trigger is exact FULL (`freeBagSpace <= 0`). Multiple full children are selected by fixed slot order CON1..CON6.

## Route preparation
Do not use a trade move-anchor macro. MAIN and selected CON are returned to the selected train coordinate by the v1.5.9 `BeginTrainRecovery` / `HandleTrainRecovery` / `HandleRobustTravel` path. MAIN is prepared first, then CON. Both must stand still at target with AutoFight/AutoPath/riding OFF before the trade chain.

## Atomic transaction
From MAIN `trade_invite_N` through MAIN final confirm, `tradeGlobalFreeze_` blocks every non-trade gameplay action across all accounts. Snapshots continue to refresh. Death/revive/map-confirm are intentionally deferred until after the chain completes.

## Child target coordinate
`trade_invite_N.macro` is child-slot-specific. Its first MAIN click should be the custom screen coordinate that selects CON N; remaining MAIN-side clicks may open trade. Macro lines are freely addable/removable.

## After transaction
Do not force start_train from inside the transaction. Release global freeze and let the donor 1.5.9 state machine process pending death/revive/confirm/route and then normal fight recovery.
