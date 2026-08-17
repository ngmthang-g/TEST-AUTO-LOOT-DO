# PROJECT KNOWLEDGE — Item Consolidator v0.2.3

## Invariants
1. Clean Route v1.5.9 remains donor for identity/death/revive/route/train recovery.
2. MAIN sell threshold default 6, trigger `freeBagSpace <= 6`.
3. CON trade trigger exact FULL (`freeBagSpace <= 0`).
4. Child priority fixed CON1..CON6; never round-robin.
5. Runtime clicks are REAL INPUT and must pass through BĐPT `CoordinatorClick`.

## Central arbiter contract
No account may directly own the physical mouse. It submits an action request. BĐPT serializes the physical input resource, sets `coordinatorInputFreeze_`, grants the target PID one click lease, executes the raw input routine, records RESULT, then clears the freeze. During the lease other accounts do snapshot-only work.

Transaction HOLD and input FREEZE are different concepts. MAIN + selected CON may stay HOLD for workflow ordering, while global input FREEZE exists only around one physical click. This permits unrelated accounts to progress between click leases without allowing mouse collisions.

## Role UI
MAIN: sell-sequence editor + shared MAIN trade editor.
CON: child-specific trade editor + selector point. Main screen hides coordinate tables until the relevant button is opened.

## Sequence model
- `mainTradeSequence_`: shared MAIN actions, configured once.
- `AccountProfile.childTradeSequence`: one plan per CON.
- Child step target=CON: owns its own point.
- Child step target=MAIN: `mainRef` points to one shared MAIN step; BĐPT executes that step on MAIN's game HWND.
- Transfer-item step is CON-only.

## Safety
Foreground mismatch, missing HWND, busy arbiter, invalid point or invalid MAIN reference must fail closed. Do not introduce a secondary SendInput path outside the arbiter.
