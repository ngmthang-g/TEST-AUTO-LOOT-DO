# VERSION v0.2.7-R2

## A. Identity / Lineage
- Version: v0.2.7-R2
- Date: 2026-08-17
- Based On: exact clean v0.2.7 commit `1308b28bd38fb044b9fceed3671820e45fb2cd23`
- Reason Created: minimal requested redesign of trade rendezvous/grouping/AutoFight-stop behavior while preserving F4 and unrelated stable v0.2.7 code.
- Last Known-Good: subsystem-specific; F4 clean-v0.2.7 behavior is protected. Complete R2 is not runtime-tested.
- Regression From: old v0.2.8/v0.2.9 were user-reported broken and are explicitly not inherited.
- Supersedes: no main merge; clean-v0.2.7 derivative.
- Related REQ: REQ-R2-001..008
- Related BUG: BUG-R2-001..005

## B. User Requests
1. Check DỒN ĐỒ toggle for real effect.
2. FULL CON immediately selects MAIN+CON, stops fight as possible, and sends both to one user-captured TỌA GD.
3. Whichever arrives first waits with ordinary train/path behavior suppressed; trade clicks start only after both arrive.
4. Group/repeat one or more consecutive trade clicks as a mini-sequence.
5. Delete per-CON selector-coordinate feature completely.
6. Failed AutoFight-stop attempts must not deadlock movement; travel and retry around map transition.
7. Merge click AUTO and old DỪNG AUTO 1 into one `AUTO` point.
8. Do not change/break F4.

## C. State Before Modification
- DỒN ĐỒ in clean v0.2.7 already gates `TickTradeCoordinator`, aborts active trade on OFF and supports independent training/selling when OFF.
- Trade preparation reused normal training targets and required per-CON `tradeSelectPoint` on MAIN.
- Six click slots duplicated AUTO / StopAuto1 at the same physical point.
- Several movement guards could end in manual-wait after repeated AutoFight-stop failure.
- No grouped row-repeat metadata existed.

## D. Investigation / Root Cause
- Toggle cosmetic-bug hypothesis: **DISPROVEN**; source shows real scheduling effect. R2 only hardens cleanup for new hold state.
- Rendezvous mismatch: **CONFIRMED**; clean active preparation was tied to train target/recovery state.
- Per-CON coordinate dependency: **CONFIRMED** in profile/UI/runtime.
- Stop-fight deadlock: **CONFIRMED** in terminal retry paths.
- First edit pass regression: CI #56 proved `PeriodicConfirmBusy()` and `HandleFightClicks()` were accidentally cut by an over-broad deletion surrounding StopAuto1; restored from clean source, not rewritten.

## E. Changes Made
- Added one global TỌA GD Map/X/Y capture + persistence.
- Added dedicated rendezvous travel state and simultaneous MAIN+CON HOLD/travel/wait.
- Added grouped trade-row repetition.
- Removed all `tradeSelectPoint`/`TradeSelect*` active code and persistence.
- Reduced point array 6→5 and renamed merged slot to AUTO.
- Added bounded AutoFight-stop fallback for movement paths.
- Protected F4 with rehydrate-time exact block comparison.
- Restored untouched clean helper functions byte-for-byte after compiler evidence.

## F. Important Implementation Details
- Clean controller SHA256: `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`
- Requested main R2 patch intermediate SHA256: `2124f79119754abfa95f320481b878239ae38c12810c7c85dbe963c44c41f09b`
- Final controller SHA256 after clean-helper restoration: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`
- Main patch SHA256: `6854233f99cf2b54bb7c1d235ec3dfc089a95752799ec4b29ba12cc83d0309b7`
- Main patch archive SHA256: `548cfaee5ca5e1426cbbe2517bd2faac9e05b0cc8a5852bc2d6ed3a2a3425952`
- Helper restoration patch SHA256: `4a8c9df615e7dc7384b0e64cb02ec3e20db8fbe2f43228dbf56cd6bd3998a6c4`
- Helper restoration archive SHA256: `70b9aaf61ca6b9383ed6ba4d4dd3a82afe9d5664368e415b119b9e6f07aaf782`
- TỌA GD tolerance default: 120.
- FULL is only child-entry gate; fixed CON1→CON6 remains.
- Five click slots: Confirm, Revive, AUTO, Attack, StopAuto2.
- Exactly 2 `SendInput(` call sites remain.

## G. Files / Components Changed
- Controller via checksum-addressed patch transport only.
- Added R2 rehydrate wrapper + main patch chunks + clean-helper restoration chunks.
- Updated workflow/knowledge/docs.
- No old v0.2.8/v0.2.9 code imported.

## H. Build / CI History
- Local patch apply test: PASS.
- CI #55 / run `32043363140`: FAILED at final SHA because Windows `git apply` produced CRLF; calculated result matched pure EOL transformation. Fix: canonical LF normalization.
- CI #56 / run `32043415457`: rehydrate and Configure x64 PASS; BUILD FAILED with missing `PeriodicConfirmBusy` and `HandleFightClicks`. Fix: exact clean-v0.2.7 helper restoration.
- CI #57 / run `32043612053`, job `95427111307`: **CI PASS** — rehydrate, x64 configure/build, route/rotation/trade tests, requested-only static audit and artifact upload all PASS.
- Final BUILD: **BUILD PASS**.
- Runtime remains **RUNTIME UNTESTED**.

## I. Runtime Result
- **RUNTIME UNTESTED**
- Awaiting: FULL trigger, different-map travel, first-arrival wait, old AutoPath suppression, failed-fight fallback/map retry, one-row and multi-row groups, DỒN OFF cleanup, F4 during travel/sequence.

## J. Regression / Revert / Failed Attempts
- Old v0.2.8/v0.2.9: rejected as donors for R2.
- Initial R2 delete range: accidentally removed two stable helpers; compiler evidence caught it and the helpers were restored byte-exact.
- Windows EOL mismatch: transport/checksum issue only; normalized without changing source logic.

## K. Known-Good Established
- F4 clean-v0.2.7 behavior is user-confirmed useful and protected.
- Complete R2: no KNOWN-GOOD status until live runtime evidence.

## L. Remaining Bugs / New Knowledge / Decisions
See BUG_REGISTRY, DECISIONS, EVIDENCE_REGISTRY.

## M. Handoff
Read Protocol V2 → PROJECT_KNOWLEDGE → this file → TRADE_RENDEZVOUS feature → BUG/DEC/EVID → exact controller.
Do not redesign F4, BĐPT, sell freeze or untouched donor helpers before collecting R2 runtime evidence.
