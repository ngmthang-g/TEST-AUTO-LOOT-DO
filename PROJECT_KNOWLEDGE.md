# PROJECT KNOWLEDGE — v0.2.5

## Persistent sell sequence lease
`coordinatorSequenceFreeze_` + `coordinatorSequenceOwnerPid_` là lease riêng cho chuỗi bán. Nó khác transient click lease. Khi MAIN ở sellPhase 6, `AcquireCoordinatorSequenceFreeze` đặt `coordinatorInputFreeze_=true` và giữ nguyên qua mọi delay/click. Chỉ owner PID được TickAccount; các acc khác bị freeze. `CoordinatorClick` của owner không unfreeze sau từng click.

Lease được release khi macro bán hết dòng, click fail/abort, StopAccount owner, hoặc REC takeover.

## Editor focus invariant
Multi-selection chỉ phục vụ copy. Dòng chỉnh sửa phải lấy từ `FocusedSelectedRow`, ưu tiên `LVNI_FOCUSED` đang selected rồi mới fallback selected đầu tiên. WM_NOTIFY trade editor nạp chính `NMLISTVIEW::iItem` vừa được selected. Sell editor cũng ép focus vào item vừa click trước khi load form.

## REC takeover
StopChecked aborts active trade if stopped account is MAIN/CON participant. StartRecorder can recover stale coordinatorInputBusy_, abort a hanging trade transaction, and release a persistent sequence lease before enabling `coordinatorRecording_`.

## Existing invariants
MAIN <=6 sell priority; FULL-only CON; fixed CON1..CON6; role-specific sequence editors; shared MAIN trade coordinates; REC/manual coordinate input; BĐPT mandatory REAL INPUT backend.
