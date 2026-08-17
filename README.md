# Thần Long Item Consolidator v0.2.2

Tool Windows x64 điều phối 1 acc MAIN và tối đa 6 acc CON trên nền Clean Route v1.5.9.

## Thay đổi quan trọng v0.2.2

### 1. Quay lại REAL INPUT chiếm chuột
Background `PostMessage` đã không click được ổn định trong game thực tế. v0.2.2 quay lại cơ chế input của donor 1.5.9:

`SetForegroundWindow -> BringWindowToTop -> SetCursorPos -> SendInput LEFTDOWN/LEFTUP`

Vì chuột vật lý là tài nguyên dùng chung, **BỘ ĐIỀU PHỐI TRUNG TÂM** chỉ cho một cửa sổ game thao tác tại một thời điểm. Tool không cố click đồng thời nhiều acc.

### 2. Tool chính + các acc là nhánh
Phía trên danh sách acc có dòng trạng thái điều phối tổng. Ví dụ:

`ĐIỀU PHỐI: MAIN đang tắt Auto/về bãi -> CON2 đang về bãi -> chuột -> MAIN -> MAIN <-> CON2 bước 3/7`

Mỗi acc bên dưới vẫn có trạng thái riêng nhưng quyền thực hiện action thuộc coordinator trung tâm.

### 3. Gán tọa độ chọn từng CON
Chọn một acc đã gán vai trò `CON1..CON6`, sau đó bấm **LẤY F8** ở mục `Tọa chọn CON`.

- Tool yêu cầu MAIN tồn tại.
- Đưa chuột tới vị trí nhân vật CON tương ứng trên cửa sổ MAIN.
- Nhấn F8.
- Tọa độ được lưu vào profile của CON theo RoleID, kèm kích thước client để scale khi cửa sổ thay đổi.
- Nút **TEST** dùng REAL INPUT để thử click tọa độ đó trên MAIN.

Khi CON đó FULL, sau khi MAIN và CON đã đứng đúng bãi train, MAIN sẽ click tọa độ này để chọn CON rồi mới chạy chuỗi giao dịch.

### 4. CHUỖI CLICK GD thay cho NẠP MACRO
`NẠP MACRO` ở v0.2.1 chỉ có nghĩa là đọc lại các file `macros/*.macro` mà người dùng tự sửa bằng Notepad. Nó không phải chức năng ghi thao tác.

v0.2.2 dùng nút **CHUỖI CLICK GD**. Cửa sổ editor cho phép:

- `+ THÊM` bước click;
- `- XÓA` bước;
- `LÊN / XUỐNG` đổi thứ tự;
- chọn bước chạy trên `MAIN` hoặc `CON đang được giao dịch`;
- loại `CLICK` hoặc `CHUYỂN ĐỒ`;
- mô tả bước;
- delay sau click;
- số lần lặp;
- `LẤY TỌA (F8)`;
- `TEST DÒNG` bằng REAL INPUT;
- `LƯU DÒNG`.

Loại `CHUYỂN ĐỒ` luôn chạy trên CON và số lần click thực tế còn bị giới hạn theo sức chứa an toàn còn lại của MAIN.

## Logic điều phối

1. Snapshot đọc tất cả acc.
2. Nếu MAIN còn `<= 6` ô trống -> ưu tiên bán đồ, không bắt đầu giao dịch.
3. CON chỉ đủ điều kiện khi `FreeBagSpace == 0`.
4. Nếu nhiều CON FULL -> ưu tiên cố định `CON1 -> CON2 -> ... -> CON6`.
5. Coordinator khóa quyền action/chuột cho transaction được chọn.
6. MAIN tắt đánh và dùng route/recovery của Clean Route 1.5.9 để về bãi train đã tick.
7. Sau khi MAIN đứng đúng bãi, CON được chọn mới tắt đánh và về bãi.
8. Khi cả hai đã đứng im đúng vị trí, MAIN click tọa độ chọn CON đã lưu.
9. Coordinator chạy từng dòng của `CHUỖI CLICK GD`; trước mỗi dòng nó đưa đúng cửa sổ MAIN/CON lên foreground rồi mới click.
10. Trong transaction, các action khác không được chen vào giành chuột.
11. Chuỗi hoàn tất -> nhả quyền chuột -> core 1.5.9 xử lý các state tiếp theo.

## Lưu ý khi dùng REAL INPUT

Trong lúc tool đang thao tác, chuột sẽ bị di chuyển và cửa sổ game sẽ được đưa lên foreground. Đây là chủ đích của v0.2.2 để đổi lấy độ tin cậy click. Không nên sử dụng chuột thủ công chen vào giữa chuỗi đang chạy.

## Macro folder

Thư mục `macros/` vẫn được đóng gói để giữ tương thích/lịch sử v0.2.1, nhưng **luồng giao dịch v0.2.2 không phụ thuộc các file `trade_invite_*.macro` nữa**. Cấu hình chính nằm trong GUI `CHUỖI CLICK GD` và được lưu vào INI.

## Trạng thái kiểm thử

CI/compile chứng minh cấu trúc build và unit/static tests. Việc click đúng UI giao dịch, tọa độ thật và delay thật vẫn phải kiểm chứng trong game với 1 MAIN + 1 CON trước khi mở rộng 6 acc.
