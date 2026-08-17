# Thần Long Item Consolidator v0.2.7-R1

Windows x64. **R1 được dựng trực tiếp từ v0.2.7 sạch**, head `1308b28bd38fb044b9fceed3671820e45fb2cd23`, controller SHA256 `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`.

v0.2.8 và v0.2.9 **không phải đầu vào của R1 và không có code nào từ hai bản đó được tái sử dụng**. R1 chỉ thêm đúng các thay đổi được yêu cầu dưới đây; các phần còn lại tiếp tục dùng logic v0.2.7.

## Những thay đổi duy nhất trong R1

### 1. Gom nhiều dòng trong CHUỖI GD ACC CON để lặp cả nhóm
- Lặp từng dòng cũ vẫn giữ nguyên.
- Có thêm `Lặp nhóm`, `GOM DÒNG ĐÃ CHỌN`, `BỎ NHÓM`.
- Chọn từ 2 dòng liền nhau trở lên rồi đặt số lần lặp. Toàn bộ cụm chạy đúng thứ tự rồi mới sang phần còn lại của chuỗi lớn.
- Nhóm có thể chứa cả dòng ACC CON và dòng tham chiếu MAIN vì mô hình shared workflow của v0.2.7 được giữ nguyên.

### 2. FULL chỉ là điều kiện bắt đầu; mỗi CON có ngưỡng kết thúc riêng
- CON chỉ được BĐPT chọn lần đầu khi `FreeBagSpace == 0`.
- Mỗi CON có `CON dừng GD khi trống ≥ N ô` riêng.
- Khi đã vào phiên, giữ đúng CON đó qua nhiều vòng; không yêu cầu CON phải FULL lại sau mỗi vòng.
- Phiên kết thúc khi **MAIN trống <= ngưỡng bán hiện tại (mặc định 6)** hoặc **CON trống >= N ô đã đặt**. Sau đó mới nhả HOLD để core v0.2.7 tiếp tục.

### 3. TỌA ĐỘ GIAO DỊCH do người dùng GET
- Có nút `TỌA GD` để lưu Map/X/Y hiện tại làm điểm giao dịch chung.
- Khi CON đủ điều kiện bắt đầu, MAIN và CON được HOLD, tắt AutoPath đang chạy về map/train, tắt AutoFight theo 2 click đã có của v0.2.7, rồi cả hai cùng chạy đến TỌA ĐỘ GIAO DỊCH.
- Trong suốt phiên, `tradeHeld` giữ MAIN + CON khỏi TickAccount/map/train bình thường.
- Chỉ sau khi đạt điều kiện kết thúc phiên mới nhả HOLD và quay lại cơ chế map/train/bán đồ nguyên bản của v0.2.7.

### 4. Auto bán FULL là nút BẬT/TẮT
- UI đổi từ checkbox thành nút `AUTO BÁN FULL: BẬT/TẮT`.
- Chỉ thay cách thao tác UI; giá trị `profile.enableSell` và logic bán đồ nền v0.2.7 không bị đổi.

## Các phần cố ý giữ nguyên từ v0.2.7
- Một `CHUỖI GD MAIN` dùng chung + một `CHUỖI GD ACC CON` global cho CON1..CON6.
- Fixed priority CON1 -> CON6.
- BĐPT / REAL INPUT; đúng hai `SendInput` raw LEFTDOWN/LEFTUP.
- REC, LẤY TỌA, TEST, copy/paste, 6 click riêng.
- DỒN ĐỒ BẬT/TẮT và chế độ auto-train/bán độc lập khi OFF.
- Sell sequence persistent FREEZE ALL.
- Donor Clean Route v1.5.9, route/map/revive, periodic confirm và các safety cũ.
