# VERSION v0.2.4 — REC recorder + multi-row copy

## REC = nhập nhanh, không phải macro đen
- `CHUỖI CLICK BÁN ĐỒ`, `CHUỖI GD MAIN`, và `CHUỖI GD CONx` có nút `REC`.
- Bấm `REC` làm BĐPT vào trạng thái `RECORDING`: mọi automation action/click bị khóa trong lúc người dùng thao tác tay.
- Timer 10 ms theo dõi nút chuột trái; chỉ click xảy ra bên trong đúng cửa sổ game được phép mới được ghi. Click vào tool, desktop, app khác hoặc CON khác bị bỏ qua.
- Bấm `DỪNG REC`: toàn bộ click vừa ghi được chuyển ngay thành các dòng tọa độ editable hiện có. REC không xóa dòng cũ; dòng mới được thêm cuối chuỗi.
- Delay của mỗi dòng được suy ra từ khoảng thời gian giữa hai click liên tiếp; dòng cuối dùng default 500 ms (giao dịch) hoặc 600 ms (bán đồ). Repeat mặc định = 1.
- Recorder chỉ ghi click trái; không cố ghi keyboard, drag hay scroll. Các loại đặc biệt như `CHUYỂN ĐỒ` có thể đổi thủ công sau khi REC.

## REC trong workflow CON
- REC của `CHUỖI GD CONx` chấp nhận click tay trên đúng CON đang edit và trên MAIN.
- Click CON trở thành dòng riêng của CON.
- Click MAIN được tra trong thư viện `CHUỖI GD MAIN`; nếu đã có đúng tọa độ thì tái sử dụng `MAIN #n`, nếu chưa có thì tự tạo một dòng MAIN dùng chung rồi CON tham chiếu nó.
- Vì vậy tọa MAIN vẫn là cấu hình dùng chung cho tất cả CON.

## Sao chép nhiều dòng
- Hai editor bán đồ và giao dịch đều bỏ chế độ single-select.
- Có thể Ctrl/Shift chọn 1 hoặc nhiều dòng -> `SAO CHÉP` -> `DÁN`.
- `DÁN` thêm bản sao vào cuối chuỗi để không làm thay đổi reference của các dòng đã tồn tại.
- Clipboard giao dịch chỉ dán giữa cùng loại editor: MAIN -> MAIN hoặc CON -> CON. Clipboard CON có thể dùng khi mở CON khác.

## Lấy 6 click của acc khác
- Mục `6 CLICK RIÊNG ACC` có nút `LẤY 6 CLICK CỦA ACC...`.
- Menu chỉ liệt kê các client khác đang quét được và đã có ít nhất một trong sáu điểm: Xác nhận ra map, Đầu thai, Auto, Đánh quái, Dừng Auto 1, Dừng Auto 2.
- Chọn acc nguồn sẽ copy các điểm hợp lệ sang acc hiện tại. Điểm nguồn chưa gán không xóa/ghi đè điểm đích.
- ClickPoint giữ base width/height nên runtime vẫn scale tọa độ theo kích thước cửa sổ acc đích.

## BĐPT invariant
- REC không tạo đường automation bypass BĐPT. Trong RECORDING, `CoordinatorClick` từ chối cấp click lease.
- Khi REC dừng, BĐPT UNFREEZE và scheduler tiếp tục.
- Runtime click tự động vẫn chỉ có một đường REAL INPUT qua `PerformRealInputClickDirect` được BĐPT cấp lease.
