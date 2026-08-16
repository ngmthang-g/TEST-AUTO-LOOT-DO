# VERSION v0.1.0 — Item Consolidator Foundation

## Goal captured
Build a coordinator for one MAIN and 1..6 CHILD accounts. All accounts train. CHILD transfers items when free slots approach the configured threshold. MAIN receives and sells when its free slots fall below the MAIN threshold. Only one trade can run at once. Actions are pure background clicks, do not occupy the physical mouse and scale with game-window size.

## Implemented
- repo reset from unrelated RemoteLoot PoC;
- one Windows x64 EXE, no bridge DLL/injection;
- visible game-window discovery;
- MAIN + ordered 1..6 CHILD assignment;
- `post` / `send` background click engine;
- normalized coordinate scaling per click;
- external `sleep/click/grid` macro DSL;
- safe `UNCONFIGURED` macro placeholders that fail closed;
- bag geometry wizard;
- visual empty-slot calibration + bag scan;
- uncertainty guard;
- separate MAIN/CHILD thresholds;
- dynamic per-trade grid-click cap from MAIN free capacity;
- round-robin CHILD scheduler;
- global trade/sell transaction mutex;
- MAIN-only sell path;
- rescan after trade;
- optional visual death detector + revive/return macro path;
- Windows x64 GitHub Actions packaging.

## CI result
- initial compile exposed legacy Win32 `near` macro collision;
- second compile exposed legacy Win32 `min/max` collisions;
- compatibility header now undefines `near/min/max` after the forced Windows include;
- final commit `2a2b7ea4f2c2ef1e2511bb820a2754e47be19fab`;
- Actions run `31961357854` (#17): configure PASS, build PASS, stage PASS, artifact upload PASS;
- artifact `ThanLongItemConsolidator-v0.1.0-win-x64` ID `9267326722`.

## Runtime still unverified
- Unity acceptance of `PostMessage`/`SendMessageTimeout`;
- exact live UI macro coordinates;
- bag capture/geometry/threshold accuracy;
- trade popup/confirm timing;
- MAIN sell sequence;
- death signature and recovery clicks.

## Next evidence
Start with one MAIN + one CHILD and prove background clicking + normalized scaling before filling the longer trade/sell/revive macros.
