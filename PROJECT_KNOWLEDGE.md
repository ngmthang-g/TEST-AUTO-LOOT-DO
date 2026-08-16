# PROJECT KNOWLEDGE

## Project Identity
- Name: RemoteLoot PoC
- Repository: `ngmthang-g/TEST-AUTO-LOOT-DO`
- Primary branch: `main`
- Current version: `v0.1.0`
- Platform: Windows x64, native C++20/CMake
- Runtime state: `RUNTIME UNTESTED`

## Project Goal
Prove one narrow question before building a larger tool: **can the frozen Thần Long client/server accept semantic loot interaction/pickup while the local character remains farther away than the built-in normal pickup flow?**

## Current State
### Source implemented
- process discovery via loaded `GameAssembly.dll`;
- per-PID shared-memory controller/bridge protocol;
- `WH_GETMESSAGE` bridge injected on the target window thread;
- IL2CPP export/class/method discovery by semantic names;
- read-only Unity `SynchronizationContext` validation;
- read-only loot API signature dump;
- read-only nearest-pack probe when `GetNearestItemPack` is zero-arg at runtime;
- one-shot `ClickToObject(RoleID)` test with no PoC movement call;
- one-shot `PickUpItemFromItemPack(itemPackID,-1,1)` test with no PoC movement call;
- read-only `HasBuff(30008009)` test.

### Runtime-confirmed working
None yet.

### Built but runtime-untested
All v0.1.0 behavior until CI/runtime evidence says otherwise.

## Hard Rules
1. This repository is an **independent RemoteLoot proof tool**, not a branch of Auto Train/Auto Sell.
2. Do not add automatic movement to remote pickup tests.
3. Do not add auto-loop/spam until one-shot server acceptance is established.
4. Do not claim direct remote pickup PASS from a successful method invocation alone.
5. PASS requires concrete runtime state: character stays put, target pack changes/disappears, bag/item state changes correctly, and no disconnect/crash.
6. A crash/disconnect can be an execution-boundary failure and must not be silently interpreted as server rejection.
7. Càn Khôn Hồ mechanism remains UNKNOWN. Only the built-in `HasBuff(30008009)` skip guard is VERIFIED from shipped source.
8. Do not broad reverse-engineer the client. Use the canonical knowledge repo first and only investigate exact missing facts.

## Verified Client Facts Used
From canonical knowledge:
- `Game.GetNearestItemPack(...)` / `Game.GetNearbyItemPack(...)` exist for item-pack discovery.
- item packs expose at least `Type`, `RoleID`, `Position` in shipped Lua.
- normal shipped auto pickup uses `MoveToEx` when distance >100, then `ClickToObject(RoleID)`.
- shipped pick-all is `Game.PickUpItemFromItemPack(itemPackID,-1,1)`.
- built-in auto pickup skips while `Game.HasBuff(30008009)` and mentions Càn Khôn Hồ.

## Important Unknowns
- Is `ItemPack.RoleID` identical to the `itemPackID` expected by direct pickup in all runtime cases?
- Does server accept direct pickup when farther than normal pickup range and buff 30008009 is absent?
- Does server acceptance change when buff 30008009 is present?
- Does Càn Khôn Hồ use this same request path or a separate server-driven subsystem?
- Is direct invocation from the validated message-hook context stable enough for this one-shot proof on the target build?

## Current Test Order
1. Validate Unity managed context.
2. Resolve exact runtime loot method signatures.
3. Scan nearest pack if signature is supported.
4. Record buff 30008009 absent/present.
5. At >100 distance, test `ClickToObject(RoleID)` with no movement call.
6. At >100 distance, test `PickUpItemFromItemPack(candidate,-1,1)` with no movement call.
7. Record pack/bag/movement/disconnect result.
8. Repeat under the opposite Càn Khôn Hồ buff state.

## Evidence Index
- `EVID-001`: canonical shipped Lua/API knowledge establishes normal loot flow and direct semantic pickup call.
- Runtime evidence for v0.1.0: pending user test.

## Decisions
- `DEC-001`: keep v0.1.0 one-shot and movement-free.
- `DEC-002`: refuse to guess unsupported runtime signatures; print them and stop that probe.
- `DEC-003`: if remote pickup passes, production implementation must use a proper action gate/MainThread dispatcher/state proof rather than preserving PoC shortcuts.
