# PROJECT KNOWLEDGE — v0.2.6

## Global consolidation mode
`tradeEnabled_` is the persisted global consolidation-mode flag (`Global/TradeEnabled` for backward compatibility).
- ON: existing MAIN/CON consolidation rules apply.
- OFF: `TickTradeCoordinator` is inert and every running account is treated as an independent auto-train/sell account. Full-bag sell trigger is `FreeBagSpace <= 0` for all roles; MAIN <=6 threshold is ignored in OFF mode.

Toggling OFF aborts any active trade and releases trade holds. It does not cancel an already-running sell workflow.

## Independent sell invariant
Auto-sell uses the same donor route/recovery and BĐPT path as before. The physical sell macro still obtains `coordinatorSequenceFreeze_` at sell phase 6 and keeps FREEZE ALL through the entire sell click sequence, including delays.

## Sell macro portability
The sell editor is accessible for every selected account. `CopySellSequenceFromAnotherAccount` replaces the target profile's `sellMacro` with the source profile's full vector after confirmation. ClickPoint base dimensions are preserved for runtime scaling.

## Existing invariants
REC remains configuration-only and freezes automation. All automation physical clicks still pass through `CoordinatorClick`/REAL INPUT. Fixed CON1→CON6 priority remains when consolidation is ON.
