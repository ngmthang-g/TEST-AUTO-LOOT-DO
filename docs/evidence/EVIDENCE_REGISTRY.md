# EVIDENCE REGISTRY

## EVID-R2-001
- Type: SOURCE
- Source: exact clean v0.2.7 controller
- Observation: SHA256 `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`.
- Supports: clean-base identity.
- Does NOT Prove: runtime success.

## EVID-R2-002
- Type: DIFF / SOURCE
- Source: R2 patch apply-test
- Observation: patch `6854233f99cf2b54bb7c1d235ec3dfc089a95752799ec4b29ba12cc83d0309b7` transforms exact base into controller `2124f79119754abfa95f320481b878239ae38c12810c7c85dbe963c44c41f09b`.
- Supports: deterministic requested-only source transport.
- Does NOT Prove: compile/runtime success.

## EVID-R2-003
- Type: USER_RUNTIME
- Source: current user request
- Observation: F4 pause is explicitly described as “rất hay” and must not change/break.
- Supports: DEC-R2-004.
- Confidence: CONFIRMED.

## EVID-R2-004
- Type: SOURCE DIFF
- Observation: clean and R2 `ToggleGlobalPause()` blocks are byte-identical; RegisterHotKey F4 line count remains one.
- Supports: protected F4 preservation.
- Does NOT Prove: all live F4 edge cases after R2 until runtime test.

## EVID-R2-005
- Type: CI
- Status: PENDING until branch build completes.
