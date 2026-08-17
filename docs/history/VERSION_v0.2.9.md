# VERSION v0.2.9 — party-open + hard rendezvous lock + F4 fallback

- Adds global MAIN TỌA TỔ ĐỘI captured by F8.
- Fixed order per round: GD LOCK -> click party -> click active CON face -> shared trade sequence.
- Trade rendezvous state is fully separated from train recovery state.
- Active MAIN/CON cannot resume train/map AutoPath until trade finishes or aborts.
- AutoPath reactivation during a transaction is stopped before any further physical click.
- MAIN sell during persistent child drain no longer returns to training; it returns directly to TỌA GD.
- F4 supports both WM_HOTKEY and keyboard-state edge fallback with debounce.
- Preserves v0.2.8 group-repeat and persistent child-drain semantics.

Controller SHA256: `5b8b4d02d7f9f12bcd49541bb32d177d3a3b01de6ed674dd66f724a5734daacc`
Patch SHA256: `68bd3ca2263ef0d2762addce0aa9fad7f0b4c316994163ffbcfa4e52f1a8d6d7`
Patch archive SHA256: `64ace31be608efb44f8407c8e03855f73c2580ba05b07c877436e00a2d747b68`
