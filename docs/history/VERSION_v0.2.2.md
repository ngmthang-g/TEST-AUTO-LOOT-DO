# VERSION v0.2.2 — REAL INPUT + Central Coordinator

## Yêu cầu người dùng
- Bỏ background PostMessage vì live game không nhận click ổn định.
- Quay về click chiếm chuột giống Clean Route v1.5.9.
- Khi nhiều acc cần action, tool phải điều phối tuần tự, không để nhiều cửa sổ tranh chuột.
- Tool phải thể hiện rõ một bộ điều phối chính; các acc là các nhánh bên dưới.
- Thêm tọa độ riêng cho từng CON để MAIN click chọn đúng CON trước giao dịch.
- Bỏ trải nghiệm `NẠP MACRO` khó hiểu; thêm GUI chuỗi click giao dịch có thể thêm/xóa/sắp xếp/lấy tọa/test.

## Thiết kế đã triển khai

### REAL INPUT
Runtime click dùng:
`SetForegroundWindow -> BringWindowToTop -> SetCursorPos -> SendInput LEFTDOWN/LEFTUP`.

Nếu cửa sổ mục tiêu không lên foreground đúng, action fail-closed để tránh click nhầm cửa sổ.

### Central coordinator
Một transaction giữ global action/mouse lock từ lúc MAIN bắt đầu tắt Auto/return-to-train cho tới khi chuỗi click giao dịch hoàn thành. Các acc khác không được phát action tranh chuột trong khoảng này.

### Per-CON selector point
Mỗi CON1..CON6 lưu `TradeSelectX/Y/W/H` trong profile RoleID. Điểm này được capture trên client MAIN bằng F8 và dùng để MAIN chọn đúng nhân vật CON trước chuỗi giao dịch.

### Visual TradeSequence
Cấu hình lưu trong `[TradeSequence]` của INI.
Mỗi bước có target MAIN/CON, kind CLICK/CHUYỂN ĐỒ, description, X/Y/W/H, delay và repeat.
Editor hỗ trợ thêm/xóa/lên/xuống/lưu/lấy F8/test từng dòng.

### Bag/priority rules giữ nguyên
- MAIN free bag <= 6 -> bán, không bắt đầu trade.
- CON chỉ eligible khi free bag == 0.
- Nhiều CON full -> CON1 đến CON6 cố định.
- MAIN prepare trước, selected CON prepare sau bằng Clean Route v1.5.9.

## Cơ chế cũ `NẠP MACRO`
Ở v0.2.1, `NẠP MACRO` chỉ reload các file `.macro` sau khi người dùng tự sửa ngoài bằng text editor. v0.2.2 không dùng đó làm luồng chính nữa; active trade dùng visual TradeSequence.

## Build integrity
CI phải:
1. rehydrate exact Clean Route v1.5.9;
2. apply v0.2.1 verified patch + compile fix;
3. verify v0.2.1 controller SHA;
4. join/apply v0.2.2 patch and verify patch SHA;
5. verify final v0.2.2 controller SHA;
6. build Windows x64;
7. run route/rotation/trade tests;
8. static audit REAL INPUT + coordinator + bag rules;
9. stage/upload artifact.

## Runtime status
CI PASS không đồng nghĩa live trade PASS. Test thực tế đầu tiên phải là 1 MAIN + 1 CON: capture selector point, tạo vài bước click, test từng dòng, rồi chạy một transaction đầy đủ trước khi tăng số CON.
