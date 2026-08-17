# Changelog

## v0.2.0
- Rebased executable foundation on supplied Clean Route v1.5.9 GUI/bridge/controller.
- Replaced v0.1 console-only startup with Win32 GUI that remains open without clients/macros.
- Added persistent MAIN / CON1..CON6 role assignment.
- Added authoritative multi-account FreeBagSpace scheduling.
- Added one-at-a-time round-robin trade coordinator.
- Added cooperative background macro runner and dynamic item-click cap.
- Migrated donor runtime clicks from SetCursorPos/SendInput to background PostMessage.
- CHILD roles can never Auto Sell; MAIN uses configurable sell threshold.
- Preserved donor death/revive/Underworld/route/train recovery state machine.
