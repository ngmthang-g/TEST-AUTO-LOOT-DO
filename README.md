# ThanLong Item Consolidator v0.1.0

Tool điều phối nhiều cửa sổ Thần Long để **dồn đồ từ acc con về một acc chính** bằng **auto-click nền thuần Win32**.

## Nguyên tắc khóa của dự án

- Chỉ `TESTauto-don-do-acc-chinh` là repo phát triển.
- Repo `clinent-game-than-long-DATA-2222` chỉ dùng làm knowledge base/tra cứu hành vi game.
- **Không inject DLL. Không hook GameAssembly. Không gọi Game/Lua API. Không gửi packet nội bộ.**
- Action thực tế chỉ là click nền vào cửa sổ game bằng `WM_MOUSE...` (`PostMessage` hoặc `SendMessageTimeout`).
- Tool không di chuyển con trỏ chuột thật.
- Tọa độ macro dùng hệ 0..1, được scale lại theo kích thước client hiện tại ở **mỗi click**.
- Tối đa 1 giao dịch đang hoạt động trong toàn bộ nhóm acc.

## Luồng điều phối

1. Tool quét các cửa sổ game đang hiển thị.
2. Người dùng chọn 1 `MAIN` và 1..6 `CHILD`.
3. Thứ tự acc con đã chọn chính là slot giao dịch 1..6.
4. Tất cả acc có thể chạy macro `move_anchor` rồi `start_train`.
5. Theo chu kỳ, tool mở/sắp xếp túi bằng macro và **scan hình ảnh 90 ô túi**.
6. Khi một CHILD còn `<= 9` ô trống, acc đó được đưa vào hàng đợi dồn đồ.
7. Chỉ một CHILD được xử lý tại một thời điểm:
   - MAIN + CHILD dừng train;
   - cả hai chạy macro `move_anchor` về cùng tọa độ train cố định;
   - MAIN chạy `trade_invite_N` đúng slot của CHILD;
   - CHILD đồng ý;
   - CHILD chạy macro đưa đồ + xác nhận;
   - MAIN chạy macro nhận/xác nhận;
   - tool chờ rồi scan lại túi.
8. Nếu MAIN còn `< 9` ô trống, MAIN ưu tiên chạy `sell_main`, scan lại, quay về anchor rồi train tiếp.
9. Acc con **không bao giờ chạy auto-sell**.
10. Sau mỗi giao dịch mặc định scan lại toàn bộ acc để xếp hàng lại từ trạng thái mới.

## Điểm quan trọng về `9 ô`

Hai ngưỡng được tách riêng trong `config/tool.ini`:

- `child_trigger_free_slots=9`: CHILD bắt đầu muốn dồn đồ khi `free <= 9`.
- `main_stop_free_slots=9`: MAIN dừng nhận khi `free < 9` và chuyển sang bán.
- `max_transfer_clicks_per_trade=9`: giới hạn số item-grid click trong một giao dịch; runtime còn tự giảm tiếp theo dung lượng MAIN để tránh nhét mù quá số ô còn trống.

Điều này bám đúng yêu cầu “acc con còn khoảng 9 ô thì giao dịch” và “acc chính còn trống dưới 9 ô thì dừng”. Có thể đổi độc lập.

## Background click + tự scale

Macro không lưu pixel tuyệt đối. Ví dụ click tại `0.80, 0.55` nghĩa là 80% chiều rộng và 55% chiều cao vùng client hiện tại. Vì vậy cùng một macro vẫn dùng được khi mỗi cửa sổ game có kích thước khác nhau, miễn layout UI giữ cùng tỷ lệ.

Hai mode:

- `post`: `PostMessage` — bất đồng bộ, nhẹ hơn.
- `send`: `SendMessageTimeout` — đồng bộ hơn, dùng khi client bỏ sót click `post`.

Cả hai đều không dùng `SetCursorPos`, không dùng `SendInput`, không lấy chuột thật.

> Rủi ro runtime cần test: một số Unity/InputSystem không nhận `WM_MOUSE...` như UI Win32 thông thường. Nếu client này không nhận background window message, phải chứng minh một đường click nền khác vẫn không chiếm chuột; không được âm thầm đổi sang `SendInput` vì sẽ phá yêu cầu.

## Macro DSL

Trong `macros/*.macro`:

```text
sleep <milliseconds>
click <x> <y> [repeat=1] [interval_ms=120] [after_ms=0]
grid <left> <top> <cols> <rows> <step_x> <step_y> <count> [interval_ms=120] [after_ms=0]
```

Ví dụ:

```text
click 0.82 0.91 1 120 400
sleep 600
grid 0.61 0.27 10 9 0.037 0.061 60 100 300
```

`grid` rất phù hợp cho thao tác click nhiều item trong túi hoặc chuỗi bán lặp lại. Số click, delay, repeat đều sửa trực tiếp được mà không cần build lại EXE.

## Các macro bắt buộc

- `start_train`
- `stop_train`
- `move_anchor`
- `bag_open_sort`
- `bag_close`
- `trade_invite_1` ... `trade_invite_6`
- `trade_accept_child`
- `trade_give_items_child` — nên dùng lệnh `grid`; coordinator sẽ áp dynamic cap theo free slots của MAIN
- `trade_confirm_child`
- `trade_confirm_main`
- `sell_main`
- `revive_return`

`revive_return.macro` đi cùng detector chết bằng visual signature; mặc định detector `enabled=0` cho tới khi capture một mẫu màn hình chết thật để tránh false positive. Khi match: recovery macro → move anchor → bật train → rescan.

## Scan túi — pure visual

Vì project không gọi nội bộ, v0.1.0 không dùng `Game.GetFreeBagSpace()` để điều khiển. Scanner làm như sau:

1. `bag_open_sort` mở túi và sắp xếp.
2. Capture client window bằng GDI/`PrintWindow`.
3. Duyệt lưới `cols x rows` (mặc định 10x9 = 90 ô).
4. So màu + variance vùng giữa từng slot với mẫu “ô trống” đã calibration.
5. Nếu quá nhiều slot nằm sát threshold, kết quả bị đánh dấu `uncertain` và **không được dùng để ra quyết định giao dịch/bán**.

### Calibration geometry

Nếu chưa có `config/bag_geometry.txt`, tool có wizard setup một lần:

- trỏ chuột vào tâm ô đầu tiên;
- trỏ vào ô kế bên phải;
- trỏ vào ô hàng dưới;
- tool tự tính `grid_left/grid_top/step_x/step_y` theo hệ 0..1.

Wizard chỉ dùng chuột lúc **setup**; runtime auto vẫn không chiếm chuột.

Sau đó lần chạy đầu tool yêu cầu một ô `calibration_row/calibration_col` đang trống rồi tự lưu mẫu vào `config/bag_calibration.txt`.

## Điều phối / chống đua trạng thái

- `transactionMutex` khóa toàn bộ chuỗi trade/sell.
- Acc con khác dù túi đầy vẫn tiếp tục train/chờ tới lượt.
- Sau trade mặc định `rescan_all_after_trade=1`.
- Queue chọn CHILD theo round-robin để tránh một acc luôn chen trước.
- Nếu scan túi không chắc chắn, acc đó không được đưa vào giao dịch.
- Nếu cửa sổ bị minimize/mất, automation dừng thay vì click vào HWND stale.
- Số item click mỗi trade được giảm động theo dung lượng MAIN để giảm nguy cơ overflow trước lần rescan.

## Auto chết / đầu thai / ra map

Có visual death detector dùng một vùng UI nhỏ đã calibration. Khi phát hiện match:

```text
stop_train
-> revive_return
-> chờ recovery
-> move_anchor
-> start_train
-> rescan all
```

Mặc định tắt vì repo hiện chưa có mẫu màn hình chết cụ thể. Bật trong `[death]` sau khi chọn vùng ổn định và capture mẫu thật.

## Build

Windows x64 / C++20 / CMake:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

GitHub Actions tạo artifact:

`ThanLongItemConsolidator-v0.1.0-win-x64`

Artifact gồm EXE, `config/`, `macros/`, README và tài liệu kiến trúc.

## Trạng thái v0.1.0

### Đã code

- multi-window discovery;
- chọn MAIN + 1..6 CHILD;
- normalized background click tự scale;
- macro engine có delay/repeat/grid-click;
- visual bag scanner + calibration + uncertainty guard;
- ngưỡng riêng MAIN/CHILD;
- dynamic transfer cap theo free slots MAIN;
- single-transaction mutex;
- round-robin queue;
- flow stop train -> move anchor -> trade -> rescan;
- MAIN-only sell flow;
- full rescan after every trade;
- visual death detector + recovery path.

### Chưa runtime-verified

- client có nhận `PostMessage`/`SendMessageTimeout` ổn định hay không;
- tọa độ UI thực tế cho từng macro;
- geometry túi thực tế và threshold scan tối ưu;
- chuỗi trade chính xác theo số popup/confirm của server hiện tại;
- sell macro thực tế;
- detector chết cần calibration mẫu hình thật; code path recovery đã có nhưng chưa runtime-verified.

Không được gọi các mục trên là PASS trước khi test thực tế.
