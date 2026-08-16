# CHANGELOG

## [v0.1.0] - 2026-08-16

### Requested
- Build a very small independent RemoteLoot PoC to determine which loot action the server accepts at distance.
- Prioritize proof of direct remote pickup before building a complete tool.

### Added / Changed
- Windows x64 C++ controller and injected `WH_GETMESSAGE` bridge.
- Runtime IL2CPP semantic resolver for loot APIs.
- Unity managed-context validation.
- Nearest ItemPack probe when runtime signature is supported without guessing.
- One-shot `ClickToObject(RoleID)` test with no PoC movement call.
- One-shot `PickUpItemFromItemPack(itemPackID,-1,1)` test with no PoC movement call.
- `HasBuff(30008009)` Càn Khôn Hồ state probe.
- GitHub Actions Windows x64 build/artifact workflow.
- Project knowledge and runtime test procedure.

### Build
- CI: pending/follow-up until workflow result is recorded.

### Runtime
- Status: `RUNTIME UNTESTED`.
- Direct remote pickup: `UNKNOWN`.
- Càn Khôn Hồ remote-loot mechanism: `UNKNOWN`.

### Next Version Notes
- Do not add automation loops yet.
- First collect repeatable runtime evidence for distant `ClickToObject` and direct pick-all with buff 30008009 absent/present.
