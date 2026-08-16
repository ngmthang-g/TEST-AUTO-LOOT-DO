# EVIDENCE REGISTRY

## EVID-001 — Shipped semantic loot path
- Type: SOURCE / REVERSE_ENGINEERING
- Date / Version: 2026-08-16 / v0.1.0
- Source: canonical `clinent-game-than-long-DATA-2222` knowledge base derived from shipped Lua/client evidence.
- Observation:
  - `Game.GetNearbyItemPack(...)` / `GetNearestItemPack(...)` exist;
  - item packs expose `RoleID`/`Position` in shipped loot logic;
  - normal auto pickup uses `MoveToEx(...)` when distance >100 before `ClickToObject(RoleID)`;
  - shipped pick-all calls `Game.PickUpItemFromItemPack(itemPackID,-1,1)`;
  - built-in auto pickup skips under `Game.HasBuff(30008009)` and mentions Càn Khôn Hồ.
- Supports: using these exact semantic APIs in the PoC.
- Does NOT Prove: server accepts direct pickup at distance; Càn Khôn Hồ uses the same request path; RoleID always equals pickup itemPackID.
- Confidence: CONFIRMED for the shipped-source facts above.

## EVID-002 — Windows x64 final-code CI build
- Type: CI
- Date / Version: 2026-08-16 / v0.1.0
- Source commit: `0bc6751e8e2521904ed296ed3fcd94a5c1b68a2e`.
- GitHub Actions: run #8 / run ID `31941065682`.
- Observation: latest v0.1.0 code snapshot completed Windows x64 GitHub Actions with conclusion `success`.
- Supports: final controller/bridge code compiles under the intended toolchain and the artifact packaging workflow succeeds.
- Does NOT Prove: game runtime stability, method resolution on the target client, or server acceptance of distant pickup.
- Confidence: CONFIRMED for BUILD/CI only.

## EVID-003 — Live remote-loot acceptance
- Type: USER_RUNTIME
- Status: PENDING
- Required observation: same distant-pack condition, buff 30008009 state recorded, no PoC movement calls, pack/bag result and disconnect/crash recorded.
- Confidence: UNKNOWN until runtime test exists.
