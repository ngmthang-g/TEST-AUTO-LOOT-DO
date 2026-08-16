# VERSION v0.1.0

## A. Identity / Lineage
- Version: v0.1.0
- Date: 2026-08-16
- Based On: empty standalone test repository
- Reason Created: prove distant loot/server acceptance before building a complete RemoteLoot tool
- Last Known-Good: none
- Regression From: none
- Supersedes: none

## B. User Request
Build a **very small RemoteLoot PoC**. If direct pickup at distance passes, only then expand the tool.

## C. State Before Modification
The test repository contained no implementation. Canonical knowledge already verified the normal shipped loot flow and semantic pickup APIs, but did not prove distant server acceptance.

## D. Investigation / Root Cause
Open question, not a bug root cause:
- VERIFIED: normal shipped flow moves when distance >100 before `ClickToObject`.
- VERIFIED: shipped pick-all calls `PickUpItemFromItemPack(itemPackID,-1,1)`.
- VERIFIED: shipped auto pickup skips with buff 30008009 / Càn Khôn Hồ.
- UNKNOWN: whether server accepts direct pickup at distance without/with that buff.

## E. Changes Made
Added a standalone x64 native PoC with:
- process/window discovery;
- target-thread hook bridge;
- Unity managed-context guard;
- runtime semantic API resolution;
- nearest-pack read probe;
- one-shot no-movement `ClickToObject`;
- one-shot no-movement direct pick-all;
- buff 30008009 read probe;
- CI build artifact.

## F. Important Implementation Details
The PoC deliberately does not call `MoveTo` or `MoveToEx` in remote interaction tests. Mutations are manual one-shot operations. Unsupported runtime signatures are reported rather than guessed.

The v0.1.0 bridge directly invokes the semantic method while running inside the validated Unity synchronization-context message hook. This is a **PoC shortcut**, not the intended production action engine. A crash/disconnect therefore cannot by itself distinguish server rejection from execution-boundary failure.

## G. Files / Components Changed
Added/updated:
- `CMakeLists.txt`
- `src/Protocol.h`
- `src/Bridge.cpp`
- `src/Probe.cpp`
- `.github/workflows/build.yml`
- `README.md`
- `PROJECT_KNOWLEDGE.md`
- `CHANGELOG.md`
- project docs

## H. Build / CI History
- Initial CI: pending at document creation; final result must be updated after workflow completion.

## I. Runtime Result
- RUNTIME: UNTESTED
- Confirmed Working: none yet
- Awaiting Test: all target-game behavior

## J. Failed Attempts
None recorded yet.

## K. Known-Good Established
None.

## L. Remaining Questions
1. Exact runtime signature/instance availability of loot methods on the target build.
2. Whether nearest pack RoleID is accepted directly as `itemPackID`.
3. Remote ClickToObject acceptance at >100 distance.
4. Remote pick-all acceptance at >100 distance.
5. Behavior difference with buff 30008009 absent vs present.

## M. Handoff
Run the artifact on a live game PID, preserve the full console log, and report movement/pack/bag/disconnect observations. Do not redesign or add loops before that evidence exists.
