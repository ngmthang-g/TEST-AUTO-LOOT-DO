# Thần Long Item Consolidator v0.2.4

Tool Windows x64 điều phối 1 MAIN và tối đa 6 CON trên nền Clean Route v1.5.9. v0.2.4 giữ Central Arbiter v0.2.3 và bổ sung **REC recorder**, **sao chép nhiều dòng**, và **lấy 6 click từ acc khác**.

## REC trong chuỗi bán đồ / giao dịch
Mở `CHUỖI CLICK BÁN ĐỒ`, `CHUỖI GD MAIN` hoặc `CHUỖI GD CONx`, sau đó bấm `REC`.

Trong khi REC:
- BĐPT chuyển sang `RECORDING` và khóa mọi auto action/click để thao tác tự động không lọt vào bản ghi.
- Người dùng tự click trong game. Recorder 10 ms chỉ nhận click trái nằm trong đúng cửa sổ game được phép.
- Ở editor CON, click trên CON được ghi là bước CON; click trên MAIN tự trở thành/tham chiếu `MAIN #n` trong thư viện MAIN dùng chung.

Bấm `DỪNG REC` để chuyển bản ghi thành **các dòng tọa độ bình thường**. Mỗi dòng vẫn sửa được ACC thực hiện, mô tả, tọa độ, delay, repeat, loại CLICK/CHUYỂN ĐỒ, xóa hoặc sắp xếp như trước. REC luôn thêm dòng mới vào cuối, không xóa chuỗi đang có.

## Sao chép một hoặc nhiều dòng
Danh sách bước bán và bước giao dịch hỗ trợ chọn nhiều bằng Ctrl/Shift. Bấm `SAO CHÉP`, sau đó `DÁN` để nhân nguyên một đoạn bước. Dán được thêm cuối chuỗi để không phá các MAIN reference đã tồn tại.

## Lấy 6 click từ acc đã có
Ở mục `6 CLICK RIÊNG ACC`, bấm `LẤY 6 CLICK CỦA ACC...` rồi chọn client nguồn. Tool copy các tọa đã có của: Xác nhận ra map, Đầu thai, Auto, Đánh quái, Dừng Auto 1, Dừng Auto 2. Điểm nào nguồn chưa có sẽ không ghi đè điểm hiện tại.

## Central Arbiter vẫn là cổng bắt buộc
Automation click vẫn đi theo: request -> BĐPT cấp lease -> FREEZE ALL -> foreground đúng PID -> SetCursorPos -> SendInput -> RESULT -> UNFREEZE. Khi REC đang chạy, BĐPT không cấp automation click lease.

## Rule nghiệp vụ giữ nguyên
- MAIN `FreeBagSpace <= 6`: bán đồ ưu tiên tuyệt đối.
- CON chỉ giao dịch khi FULL (`FreeBagSpace == 0`).
- Nhiều CON FULL: CON1 -> CON2 -> ... -> CON6.
- Route/death/revive/train recovery vẫn dùng nền Clean Route v1.5.9.

## Runtime test nên làm
Bắt đầu với MAIN + CON1. Test `REC` trên chuỗi MAIN, sau đó REC một chuỗi CON có click xen kẽ giữa CON1 và MAIN. Kiểm tra các dòng sau khi DỪNG REC, thử SAO CHÉP/DÁN, rồi mới bật workflow tự động.
