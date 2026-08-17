# PROJECT KNOWLEDGE — Item Consolidator

## Current version
v0.2.0 foundation, donor = user-supplied ThanLong Clean Route v1.5.9.

## Architecture invariant
Resolver/bridge state remains authoritative for read-only state. Trade UI actions are user-recorded background click macros. One global transaction owns MAIN at a time.

## Account roles
- 0: off
- 1: MAIN
- 2..7: CON1..CON6
Roles persist in the RoleID profile (`TradeRole`). Assigning a role automatically clears the same role from another loaded account. CHILD roles cannot Auto Sell.

## Bag scheduling
- CHILD eligible: `freeBagSpace <= ChildTriggerFreeSlots` (default 9).
- MAIN blocks another trade: `freeBagSpace < MainSellThreshold` (default 9).
- MAIN Auto Sell uses the threshold when role=MAIN; legacy OFF-role behavior keeps the old full-bag trigger.
- Trade give macro gets a dynamic click cap: min(MaxTransferClicks, MAIN capacity down to threshold-1).

## Trade transaction phases
stop MAIN -> stop CHILD -> move MAIN -> move CHILD -> invite slot N -> CHILD accept -> CHILD give -> CHILD confirm -> MAIN confirm -> restart CHILD -> restart MAIN -> cooldown/rescan.

Each macro runs cooperatively one step/click at a time. Snapshot polling continues. Death or map transition aborts the transaction and releases both account holds; the donor death/route FSM resumes next tick.

## Background input
All controller runtime click paths were migrated from donor physical input to `PostMessage(WM_MOUSEMOVE/WM_LBUTTONDOWN/WM_LBUTTONUP)`. F8 coordinate capture still reads the physical cursor during setup only; runtime does not move it.

## Known unproven point
Unity background acceptance is runtime-dependent and must be proven with one MAIN + one CHILD. No fallback to `SendInput` is allowed without changing the explicit project constraint.
