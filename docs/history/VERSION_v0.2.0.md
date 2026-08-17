# VERSION v0.2.0

## User problem
v0.1 compiled but appeared not to start because it was console-first and could exit immediately on missing config/macro/client conditions. The user supplied a proven v1.5.9 auto foundation and clarified that trade itself is only a known click sequence; the hard problem is multi-account death recovery, bag-state observation and scheduling.

## Solution
Use v1.5.9 as the executable donor. Preserve GUI, bridge snapshot and death/route controller. Add MAIN/CON role persistence and a global coordinator. Replace visual bag/death inference with donor authoritative read-only snapshot fields. Run trade as external click macros.

## Click policy
Runtime input is background PostMessage only. No physical cursor movement, SendInput, or foreground stealing. F8 remains setup-only coordinate capture.

## Transaction safety
Only one child owns MAIN. Macro execution is cooperative, so state polling continues. Death/map transition aborts the transaction and returns ownership to donor recovery. Main capacity is checked before queueing and item click count is capped.

## Test status
Source integration completed; GitHub Actions Windows x64 build and runtime test are required next. Runtime background-click acceptance is not claimed until tested against the game.
