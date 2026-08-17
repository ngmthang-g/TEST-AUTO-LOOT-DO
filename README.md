# Thần Long Item Consolidator v0.2.7-R2

Windows x64. **Nền code duy nhất: exact v0.2.7 sạch** (`controller.cpp` SHA256 `397f1cf0...`). R2 không lấy code từ v0.2.8/v0.2.9 cũ.

## DỒN ĐỒ
- Clean v0.2.7 đã có toggle thật: OFF chặn trade coordinator, abort transaction và quay về auto-train/bán độc lập.
- R2 giữ cơ chế đó và bổ sung dọn sạch mọi `tradeHeld` + state TỌA GD mới khi OFF/abort.
- ON vẫn giữ MAIN `freeBagSpace <= 6` ưu tiên bán và CON chỉ được chọn lúc FULL chính xác `freeBagSpace == 0`, ưu tiên CON1→CON6.

## TỌA GD global
- Chọn một acc đang đứng đúng điểm muốn giao dịch → bấm `TỌA GD • LẤY` để lưu Map/X/Y global.
- Khi CON FULL: BĐPT HOLD **MAIN + CON ngay lập tức**, StopPath đường về bãi cũ, thử tắt AutoFight và cho cả hai cùng đi TỌA GD.
- Acc tới trước bị giữ tại điểm, không được auto-train/AutoPath về bãi cũ.
- Chỉ khi snapshot xác nhận **cả MAIN và CON đều tới TỌA GD** mới chạy chuỗi giao dịch có sẵn.
- Nếu đang fallback và đổi map, tool check/retry tắt AutoFight rồi tiếp tục route; không treo vĩnh viễn chỉ vì AutoFight chưa tắt được.

## Nhóm lặp trong CHUỖI GD ACC CON
- Chọn 1/2/3/... dòng **liên tiếp**.
- Nhập `Lặp nhóm`, bấm `GOM DÒNG ĐÃ CHỌN`.
- Mini-sequence đó chạy đủ N vòng rồi mới đi tiếp chuỗi lớn.
- `BỎ NHÓM` xóa group của các dòng chọn.
- Repeat từng dòng cũ vẫn giữ nguyên.

## Bỏ tọa độ gán riêng cho CON
Toàn bộ active UI/persistence/runtime `tradeSelectPoint` / `TradeSelect*` đã bị xóa. Không còn tọa riêng CON1..CON6.

## AUTO / DỪNG AUTO
- 6 click riêng giảm còn **5 click**.
- Click cũ `AUTO` (#3) và `DỪNG AUTO 1` (#5) cùng một điểm nên R2 chỉ giữ slot `AUTO`.
- Tắt fight dùng `AUTO → DỪNG AUTO 2`.

## F4 — PROTECTED
`ToggleGlobalPause()` và `RegisterHotKey(...VK_F4)` giữ nguyên từ clean v0.2.7. Rehydrate wrapper so sánh exact block và FAIL nếu bị thay đổi.

## Không thay đổi
Shared `CHUỖI GD MAIN` + một global `CHUỖI GD ACC CON`, REC, BĐPT, REAL INPUT, fixed CON1→CON6, sell priority, persistent FREEZE ALL của chuỗi bán, route/map/revive donor Clean Route v1.5.9 và các helper fight/confirm ổn định khác.

**BUILD/CI PASS không đồng nghĩa RUNTIME PASS. R2 vẫn là RUNTIME UNTESTED cho tới test game thật.**
