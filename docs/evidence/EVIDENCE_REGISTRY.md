# EVIDENCE REGISTRY

## EVID-R2-001
- Type: SOURCE
- Source: exact clean v0.2.7 controller
- Observation: SHA256 `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`.
- Supports: clean-base identity.
- Does NOT Prove: runtime success.

## EVID-R2-002
- Type: DIFF / SOURCE
- Observation: main requested patch SHA `6854233f99cf2b54bb7c1d235ec3dfc089a95752799ec4b29ba12cc83d0309b7`; helper-restoration patch SHA `4a8c9df615e7dc7384b0e64cb02ec3e20db8fbe2f43228dbf56cd6bd3998a6c4`; final controller SHA `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- Supports: deterministic clean-base → R2 source chain.
- Does NOT Prove: live runtime success.

## EVID-R2-003
- Type: USER_RUNTIME
- Source: user request for v0.2.7-R2
- Observation: F4 pause is explicitly described as very good and must not change/break.
- Supports: DEC-R2-004 / protected known-good F4 behavior.
- Confidence: CONFIRMED.

## EVID-R2-004
- Type: SOURCE DIFF
- Observation: clean and final `ToggleGlobalPause()` source blocks are byte-identical; both block SHA256 `17b062a700700a9392dd197e6b1dd7924ad9b490753659fecc14216b82576132`; one F4 RegisterHotKey line remains.
- Supports: source-level F4 preservation.
- Does NOT Prove: every live timing edge after R2.

## EVID-R2-005
- Type: CI
- Run: `32043363140` (#55)
- Observation: FAILED during R2 final checksum after Windows `git apply`; mismatched SHA equaled a pure LF→CRLF transformation of the intended source.
- Supports: transport/EOL failure history and LF-normalization correction.
- Does NOT Prove: source logic failure.

## EVID-R2-006
- Type: CI / COMPILER
- Run: `32043415457` (#56)
- Observation: rehydrate PASS, Configure x64 PASS, then BUILD FAILED because `PeriodicConfirmBusy` and `HandleFightClicks` definitions were absent.
- Supports: BUG-R2-005 root cause detection.
- Correction: exact clean-v0.2.7 helper definitions restored.

## EVID-R2-007
- Type: CI
- Run: `32043612053` (#57), job `95427111307`
- Head: `9b4d0337b6681dc97008f000d71d8b100bbdd738`
- Observation: **CI PASS** — exact clean-v0.2.7 rehydrate + R2 patch chain PASS, Configure x64 PASS, Build Release PASS, route/rotation/trade tests PASS, static requested-only audit PASS, artifact stage/upload PASS.
- Supports: BUILD PASS / CI PASS for the final R2 controller and audit invariants.
- Does NOT Prove: live game runtime success.
- Runtime remains: **RUNTIME UNTESTED**.
