# VERSION v0.2.7-R2

## A. Identity / Lineage
- Version: v0.2.7-R2
- Date: 2026-08-17
- Based On: exact clean v0.2.7 (`1308b28bd38fb044b9fceed3671820e45fb2cd23`)
- Reason Created: user requested a minimal, clean-v0.2.7-only redesign of trade rendezvous/grouping/AutoFight-stop behavior while protecting F4 and all unrelated stable features.
- Last Known-Good: subsystem-specific; F4 is user-confirmed protected. Complete R2 is not runtime-tested.
- Regression From: old v0.2.8/v0.2.9 experiments were reported broken and are not inherited.
- Supersedes: no mainline merge; this is a clean v0.2.7 derivative.
- Related REQ: REQ-R2-001..008
- Related BUG: BUG-R2-001..004
- Related Features: TRADE_RENDEZVOUS, TRADE_SEQUENCE_GROUPS, CLICK_ENGINE, AUTO_FIGHT_STOP_FALLBACK.

## B. User Requests
1. Verify DỒN ĐỒ actually toggles scheduling.
2. Full CON should immediately cause MAIN+CON HOLD and travel to one user-captured TỌA GD.
3. First arrival waits; both must be confirmed there before existing click sequence.
4. Group and repeat 1/2/multiple click rows.
5. Completely remove per-CON selection coordinates.
6. Do not permanently wait if AutoFight stop fails; move and retry after map changes.
7. Merge AUTO and old DỪNG AUTO 1 into one AUTO point.
8. Do not change/break F4.

## C. State Before Modification
- Clean v0.2.7 DỒN ĐỒ toggle was already functional.
- Clean v0.2.7 trade preparation drove MAIN then CON back to their normal training targets before trade.
- Each CON required a separate `tradeSelectPoint` on MAIN.
- AutoFight stop failure in train/sell/M87 paths could remain waiting for manual intervention.
- Six per-account click slots included both AUTO and StopAuto1 despite user reporting they are the same physical point.
- No multi-row group-repeat metadata existed.
- F4 worked well and must remain protected.

## D. Investigation / Root Cause
- Toggle issue: DISPROVEN as a purely cosmetic bug; source already gates scheduler and aborts active trade on OFF. R2 adds stale-HOLD cleanup.
- Rendezvous mismatch: CONFIRMED in clean source; trade preparation reused `trainRecoveryPhase`/training targets rather than a dedicated trade coordinate.
- Per-CON coordinate dependency: CONFIRMED in clean coordinator and profile persistence.
- Stop-fight deadlock: CONFIRMED in source branches that ended with “chờ thủ công” after retry limit.
- F4 regression risk: addressed by byte-comparing clean and patched function during rehydrate.

## E. Changes Made
- `controller.cpp`: global TỌA GD, dedicated trade travel state, dual hold/travel/wait, map-transition wait semantics.
- `controller.cpp`: grouped mini-sequence metadata/editor/execution.
- `controller.cpp`: removed tradeSelectPoint/TradeSelect.
- `controller.cpp`: click array 6→5; AUTO replaces old StopAuto1; StopAuto2 retained.
- `controller.cpp`: train/sell/M87/trade movement fallbacks proceed after stop-fight retries and retry after map changes.
- `tools/rehydrate_v027_r2.ps1`: exact clean base/final SHA and protected-F4 equality checks.
- CI audit: forbidden-token checks and invariants.

## F. Important Implementation Details
- Base SHA256: `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`
- Final controller SHA256: `2124f79119754abfa95f320481b878239ae38c12810c7c85dbe963c44c41f09b`
- Patch SHA256: `6854233f99cf2b54bb7c1d235ec3dfc089a95752799ec4b29ba12cc83d0309b7`
- Patch archive SHA256: `548cfaee5ca5e1426cbbe2517bd2faac9e05b0cc8a5852bc2d6ed3a2a3425952`
- TỌA GD default tolerance: 120.
- AutoFight stop timing inherits 750 ms then 1300 ms verify in train/sell/trade.
- FULL condition remains entry-only for selecting a child.
- F4 protected source block hash: clean and final compared directly by rehydrate wrapper.

## G. Files / Components Changed
- Modified through patch: generated donor controller only.
- Updated: README, PROJECT_KNOWLEDGE, CHANGELOG, workflow, .gitattributes.
- Added: R2 rehydrate wrapper, R2 patch chunks, version/feature/bug/decision/evidence docs.

## H. Build / CI History
- Local patch apply-test: PASS and byte-identical final SHA.
- CI: PENDING at document creation.
- Runtime: UNTESTED.

## I. Runtime Result
- RUNTIME: **UNTESTED**
- Awaiting test: FULL trigger, dual route, first-arrival hold, map-cross fallback, group repeat, DỒN OFF, F4 during rendezvous/sequence.

## J. Regression / Revert / Failed Attempts
- Old v0.2.8/v0.2.9 are explicitly not used as source donors for R2.
- R2 must not silently copy their state machines or F4 modifications.

## K. Known-Good Established
- F4 pause behavior: protected from clean v0.2.7 due explicit user positive runtime feedback.
- No complete R2 KNOWN-GOOD until live test.

## L. Remaining Bugs / New Knowledge / Decisions
See BUG_REGISTRY, DECISIONS, EVIDENCE_REGISTRY.

## M. Handoff
Read Protocol V2 → PROJECT_KNOWLEDGE → this file → affected feature docs → exact controller.
Do not redesign F4, BĐPT, sell freeze or unrelated donor core before collecting R2 runtime evidence.
