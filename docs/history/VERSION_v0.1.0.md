# VERSION v0.1.0 — Item Consolidator Foundation

## User request captured
Build an automation coordinator for one MAIN account and 1..6 CHILD accounts. All accounts train. CHILD accounts transfer items to MAIN when their bags approach full. MAIN receives and sells when its bag approaches full. Only one CHILD may trade at a time. Actions must be pure background clicks, must not consume the physical mouse, and must scale with game-window size.

## Implemented
- repo reset from unrelated RemoteLoot PoC;
- one EXE, no bridge DLL;
- visible-window discovery and MAIN/CHILD selection;
- 1..6 ordered target slots;
- PostMessage/SendMessageTimeout click modes;
- normalized per-click coordinate scaling;
- external editable macro files;
- configurable delay/repeat/grid clicks;
- visual bag geometry wizard;
- visual empty-slot calibration and scan;
- uncertain-scan guard;
- separate MAIN/CHILD thresholds;
- dynamic per-trade grid-click cap from MAIN capacity;
- round-robin CHILD scheduler;
- one global transaction mutex;
- MAIN-only sell flow;
- full rescan after trade;
- optional calibrated visual death detector and recovery macro path;
- GitHub Actions packaging for Windows x64.

## Not claimed as runtime PASS
- background click acceptance by current Unity build;
- live UI coordinates;
- bag scanner accuracy;
- trade timing/confirmation sequence;
- sell path;
- death detection/recovery.

## Required next evidence
1. CI green.
2. Safe background click probe on one visible client.
3. Same normalized click on two different window sizes.
4. Bag geometry/calibration versus manual free-slot count.
5. One MAIN + one CHILD trade end-to-end.
6. Scale to 2..6 CHILD and observe serialization.
