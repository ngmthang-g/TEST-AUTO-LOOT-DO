Macro DSL (all coordinates are normalized 0..1 and auto-scale with each window):

sleep <milliseconds>
click <x> <y> [repeat=1] [interval_ms=120] [after_ms=0]
grid <left> <top> <cols> <rows> <step_x> <step_y> <count> [interval_ms=120] [after_ms=0]

IMPORTANT:
- 0.0,0.0 is top-left of the game CLIENT area.
- 1.0,1.0 is bottom-right of the current game CLIENT area.
- Shipped *.macro files contain an `UNCONFIGURED` line on purpose.
- `UNCONFIGURED` is NOT valid DSL: it makes that macro fail parsing so the tool cannot pretend an empty placeholder succeeded.
- After measuring a real UI flow, replace the whole placeholder with tested sleep/click/grid steps and remove `UNCONFIGURED`.
- Keep delays conservative first; reduce them only after repeated stable tests.
