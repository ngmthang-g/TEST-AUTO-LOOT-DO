# Architecture — Item Consolidator

## 1. Mục tiêu

Điều phối một party nhiều client theo mô hình:

```text
MAIN (train + nhận đồ + bán)
CHILD 1..6 (train + cho đồ, không bán)
```

Action layer bị giới hạn cố ý ở **background window click**.

## 2. Pipeline

```text
Window Discovery
  -> Session Registry (MAIN / CHILD / tradeSlot)
  -> Visual Bag Scanner
  -> Per-session snapshot (freeSlots/reliable/state)
  -> Coordinator
  -> Global transaction mutex
  -> Macro runner
  -> BackgroundClick(HWND, normalized x/y)
  -> wait / rescan
```

Không có DLL bridge, IL2CPP resolver, remote thread hay packet sender.

## 3. State / priority

Ưu tiên:

```text
window invalid / manual stop
> MAIN needs sell
> current active trade
> next CHILD waiting transfer
> periodic bag scan
> normal train
```

Chỉ một flow có quyền “chiếm” MAIN tại một thời điểm.

## 4. Queue

CHILD eligible khi:

```text
bagReliable == true
AND freeSlots <= child_trigger_free_slots
```

MAIN có quyền nhận khi:

```text
bagReliable == true
AND freeSlots >= main_stop_free_slots
```

Nếu MAIN `< main_stop_free_slots`, sell có priority cao hơn mọi CHILD.

Mỗi trade còn có dynamic grid-click cap theo MAIN free space để tránh overflow trước lần rescan kế tiếp.

Round-robin cursor được cập nhật sau khi chọn một CHILD để tránh starvation.

## 5. Trade transaction

```text
lock transactionMutex
MAIN stop_train
CHILD stop_train
MAIN move_anchor
CHILD move_anchor
wait movement settle
MAIN trade_invite_<child.tradeSlot>
wait
CHILD trade_accept_child
wait
CHILD trade_give_items_child
CHILD trade_confirm_child
MAIN trade_confirm_main
wait
restart train
unlock
rescan all
```

Failure => không tự đoán thành công; ép rescan trước retry.

## 6. Coordinate model

Mỗi click lưu `(nx, ny)` trong `[0,1]`.

Runtime:

```text
x = round(nx * (clientWidth  - 1))
y = round(ny * (clientHeight - 1))
```

Client size được đọc lại trước **mỗi click**.

## 7. Bag visual scanner

Visual-only vì project cấm internal action/API.

- 90 slot là giả định cấu hình, không phải fact khóa cứng.
- grid geometry phải đo từ UI thật.
- calibration lấy mẫu một slot trống thật.
- mỗi slot so mean RGB + luminance variance.
- nhiều slot sát threshold => `uncertain`; coordinator không hành động từ scan đó.

Điểm yếu cần nhớ: icon/skin/theme/UI scale có thể làm thay đổi signature. Vì vậy visual scanner phải fail-closed, không fail-open.

## 8. Background click risk

`WM_MOUSE...` không bảo đảm mọi Unity build sẽ nhận như Win32 control. v0.1.0 có `post` và `send` để test hai cách không chiếm chuột. Nếu cả hai fail thì phải nghiên cứu một cơ chế input nền khác; **không được thay bằng SendInput/SetCursorPos nếu mục tiêu vẫn là không chiếm chuột**.

## 9. Death/revive roadmap

`revive_return.macro` và visual-signature detector đã có trong v0.1.0 nhưng detector mặc định tắt cho tới khi capture mẫu chết thật. Khi bật và match:

```text
visual death proof
-> abort/yield active transaction safely
-> revive_return macro
-> visual/map-ready proof
-> move_anchor
-> start_train
-> rescan bag
```

Cần ảnh/runtime calibration thật, không nên đoán vùng chết.
