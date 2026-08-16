# RemoteLoot PoC v0.1.0

Mục tiêu duy nhất của repo này là **xác định server/client chấp nhận hành vi nhặt bọc nào khi nhân vật đứng xa**. Đây chưa phải Auto Loot hoàn chỉnh.

## Căn cứ VERIFIED từ knowledge base

Client đã có semantic loot API:

- `Game.GetNearestItemPack(...)` / `Game.GetNearbyItemPack(...)`
- `Game.ClickToObject(RoleID)`
- `Game.PickUpItemFromItemPack(itemPackID, slotIndex, UsingAuto)`
- built-in pick-all: `Game.PickUpItemFromItemPack(itemPackID, -1, 1)`
- built-in auto loot bình thường: nếu khoảng cách > 100 thì `MoveToEx(...)` rồi mới `ClickToObject(...)`
- built-in auto pickup bị skip khi `Game.HasBuff(30008009)` và source hiển thị thông báo Càn Khôn Hồ.

Điểm **chưa VERIFIED** là server có chấp nhận direct pickup ở xa khi không có Càn Khôn Hồ hay không. PoC này tồn tại để trả lời đúng câu đó.

## Thiết kế PoC

PoC gồm 2 file x64:

- `RemoteLootProbe.exe` — controller console độc lập.
- `RemoteLootBridge.dll` — `WH_GETMESSAGE` hook cực nhỏ chạy trên window thread của game và gọi IL2CPP semantic API.

Không có:

- Auto Train.
- Auto Sell.
- vòng lặp auto loot.
- OCR/pixel scan.
- `MoveTo` / `MoveToEx` trong các test remote.
- danh sách 90 action hoặc spam request.

Mỗi lệnh mutable là **one-shot** do người test bấm tay.

> PoC gọi semantic action trực tiếp từ validated Unity `SynchronizationContext` hook để giảm biến số khi test server acceptance. Đây **không phải** kiến trúc action engine production cuối cùng. Nếu direct pickup PASS, bản tool thật phải quay về ActionGate/MainThread dispatcher + state proof chuẩn.

## Build

GitHub Actions tự build Windows x64 và upload artifact:

`RemoteLootPoC-v0.1.0-win-x64`

Build local:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Sau build, để `RemoteLootProbe.exe` và `RemoteLootBridge.dll` cùng một thư mục.

## Chạy

1. Mở game và đăng nhập nhân vật.
2. Chạy `RemoteLootProbe.exe` cùng mức quyền với game. Nếu game chạy Administrator thì probe cũng chạy Administrator.
3. Chọn PID game.
4. Probe tự chạy:
   - `Validate Unity managed context`;
   - `Resolve/print loot API signatures`.
5. Tạo một bọc đồ trên đất và đứng **xa hơn khoảng nhặt bình thường**.
6. Tắt Auto pickup của game.
7. Test theo thứ tự bên dưới.

## Test A — scanner

Menu `3`:

`Scan nearest ItemPack`

Nếu method runtime đúng dạng PoC hỗ trợ, tool in `RoleID` của bọc gần nhất và giữ nó làm candidate `itemPackID`.

Nếu runtime signature khác, PoC **không đoán tham số**; log sẽ in exact signature và trả `SIGNATURE_UNSUPPORTED`.

## Test B — remote ClickToObject

Menu `4`:

```text
Game.ClickToObject(ItemPack.RoleID)
```

PoC tuyệt đối không gọi `MoveTo`/`MoveToEx` trước hoặc sau lệnh này.

### PASS có ý nghĩa khi

- nhân vật không chạy lại gần;
- pack-content lifecycle hoặc pickup response xuất hiện;
- game không disconnect/crash;
- kết quả lặp lại được.

### FAIL

- không có phản ứng;
- server từ chối;
- chỉ hoạt động khi ở gần;
- disconnect/crash/exception.

Disconnect/crash **không tự động chứng minh server từ chối**; có thể là execution-boundary/re-entrancy failure. Log phải được giữ lại.

## Test C — direct pickup all ở xa

Menu `5`:

```text
Game.PickUpItemFromItemPack(itemPackID, -1, 1)
```

Không có movement call.

### DIRECT REMOTE PICKUP = PASS chỉ khi

Cùng một test condition cho thấy:

- nhân vật vẫn đứng nguyên vị trí;
- bọc mục tiêu biến mất hoặc contents của nó giảm đúng;
- tay nải/item state thay đổi đúng;
- không disconnect/crash;
- có thể lặp lại ở nhiều bọc.

Nếu PASS khi **buff 30008009 ABSENT**, giả thuyết mạnh nhất là khoảng cách >100 trong shipped Auto chỉ là client-side policy hoặc server cho phép semantic pickup từ xa trong phạm vi AOI.

Nếu FAIL khi buff absent nhưng PASS khi buff present, server nhiều khả năng có entitlement/state check liên quan Càn Khôn Hồ.

Nếu cả direct pickup lẫn ClickToObject đều không phải cơ chế khi buff present, cần chuyển sang nghiên cứu targeted subsystem của Càn Khôn Hồ; không broad reverse client.

## Test D — Càn Khôn Hồ

Menu `6` gọi:

```text
Game.HasBuff(30008009)
```

Chạy lại cùng test B/C ở hai trạng thái:

1. `ABSENT`
2. `PRESENT`

Không thay đổi điều kiện khác nếu có thể.

## Bảng ghi kết quả cần gửi lại

```text
Game PID:
ValidateContext: PASS/FAIL
Loot API signatures:
ScanNearestPack: PASS/FAIL
Distance: gần / >100 / rất xa trong AOI
Buff 30008009: ABSENT/PRESENT
ClickToObject: PASS/FAIL + hiện tượng
DirectPickupAll: PASS/FAIL + hiện tượng
Nhân vật có di chuyển: YES/NO
Pack biến mất: YES/NO
Bag thay đổi: YES/NO
Disconnect/crash: YES/NO
Log detail:
```

## Evidence status v0.1.0

- Source/CI: đang kiểm tra.
- Runtime: `RUNTIME UNTESTED`.
- Direct remote pickup: `UNKNOWN` cho tới khi có test thật.
- Càn Khôn Hồ mechanism: `UNKNOWN`; buff 30008009 skip guard là VERIFIED, nhưng cơ chế nhặt riêng của nó chưa được chứng minh.
