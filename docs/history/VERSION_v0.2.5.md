# VERSION v0.2.5 — Sell sequence FREEZE + editor/REC fixes

## FREEZE ALL bán đồ
- Chỉ áp dụng khi MAIN bước vào `CHUỖI CLICK BÁN ĐỒ` (sellPhase 6), không khóa thời gian chạy tới NPC hay quay về bãi.
- BĐPT cấp `SEQUENCE LEASE` cho MAIN một lần.
- Giữa các dòng và delay, các acc khác vẫn FREEZE.
- Mỗi click của MAIN chạy dưới lease đang giữ; không UNFREEZE sau từng click.
- Dòng cuối xong -> UNFREEZE. Click fail/abort/StopAccount/REC takeover -> release lease fail-safe.

## Editor
- Multi-select vẫn dùng Ctrl/Shift cho SAO CHÉP.
- `FocusedSelectedRow` là dòng duy nhất dùng cho sửa/LƯU/LẤY TỌA/TEST.
- WM_NOTIFY nạp đúng item vừa click.
- Sau khi dòng CON đang tham chiếu MAIN được đổi target về CON, form của chính dòng đó được reload và mở Mô tả + Loại CLICK/CHUYỂN ĐỒ.

## REC
- DỪNG AUTO participant -> AbortTrade, nhả tradeHeld.
- REC xử lý stale transient lease, hanging trade txn và persistent sell sequence lease trước khi RECORDING.
- REC vẫn chỉ ghi manual left-click và chuyển thành editable rows.
