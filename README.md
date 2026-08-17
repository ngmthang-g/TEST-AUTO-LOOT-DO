# Thần Long Item Consolidator v0.2.7

Windows x64. v0.2.7 đơn giản hóa cấu hình giao dịch: từ nhiều workflow CON riêng thành đúng **2 bộ dữ liệu dùng chung**.

## 1. CHUỖI GD MAIN
- Là thư viện tọa độ MAIN dùng chung cho mọi giao dịch.
- Các dòng MAIN #n chỉ cần sửa một lần.

## 2. CHUỖI GD ACC CON
- Chỉ còn **một workflow global duy nhất** cho CON1 -> CON6.
- Không còn `CHUỖI GD CON1`, `CON2`, ... riêng.
- Khi BĐPT chọn MAIN giao dịch với CONn, mọi dòng `ACC CON` trong workflow sẽ chạy trên đúng cửa sổ CONn đang active.
- Dòng tham chiếu `MAIN #n` vẫn chạy trên MAIN, vì workflow giao dịch có thể cần xen kẽ MAIN -> CON -> MAIN.
- `CHUYỂN ĐỒ` luôn là thao tác của active CON.

## REC / sửa chuỗi
- Mở `CHUỖI GD ACC CON` từ bất kỳ CON nào. CON đang chọn chỉ là cửa sổ donor để REC/LẤY TỌA/TEST.
- Tọa CON ghi được dùng cho toàn bộ CON1..CON6 và vẫn scale theo BaseW/BaseH.
- REC vẫn có thể ghi xen kẽ click trên CON donor và MAIN; click MAIN được ánh xạ về thư viện `CHUỖI GD MAIN`.

## Migration từ v0.2.6
Nếu chưa có section global `ChildTradeSequence`, v0.2.7 tự migrate một lần:
1. ưu tiên workflow cũ của CON có slot thấp nhất đang tồn tại (CON1 -> CON6),
2. nếu không có thì dùng template legacy cũ,
3. lưu thành `ChildTradeSequence` global.

Dữ liệu per-CON cũ vẫn được giữ trong profile để tương thích/rollback nhưng **không còn được runtime dùng**.

DỒN ĐỒ BẬT/TẮT, auto-train độc lập khi OFF, chuỗi bán, FREEZE ALL xuyên suốt chuỗi bán, BĐPT, REC, 6 click riêng, route/map/revive và donor Clean Route v1.5.9 giữ nguyên.
