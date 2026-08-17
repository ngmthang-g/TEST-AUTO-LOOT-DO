# Thần Long Item Consolidator v0.2.6

Windows x64. v0.2.6 bổ sung **DỒN ĐỒ BẬT/TẮT** và khả năng lấy nguyên **CHUỖI CLICK BÁN ĐỒ** từ acc khác.

## DỒN ĐỒ: BẬT
- Giữ logic MAIN/CON hiện tại.
- MAIN bán khi FreeBagSpace <= ngưỡng MAIN (mặc định 6).
- CON chỉ được chọn giao dịch khi FULL đúng 0 ô.
- Nhiều CON FULL giữ ưu tiên CON1 -> CON6.
- Giao dịch vẫn qua BĐPT và chuỗi GD riêng/shared MAIN như các bản trước.

## DỒN ĐỒ: TẮT
- Scheduler giao dịch MAIN↔CON dừng hoàn toàn; nếu đang có transaction thì abort và nhả HOLD.
- Mỗi acc đang RUN trở thành auto train độc lập.
- Bất kỳ acc nào FULL = 0 ô sẽ tự dừng đánh, chạy NPC bán, thực hiện chuỗi bán riêng, quay về bãi và tiếp tục train.
- Trong mode độc lập, ngưỡng MAIN <=6 không dùng; tất cả acc bán theo FULL = 0.
- Khi bắt đầu CHUỖI CLICK BÁN ĐỒ, BĐPT vẫn giữ SEQUENCE LEASE và FREEZE ALL xuyên suốt cả chuỗi như v0.2.5.

## LẤY CHUỖI BÁN CỦA ACC KHÁC
`CHUỖI CLICK BÁN ĐỒ` mở được cho mọi acc. Trong editor có `LẤY CHUỖI CỦA ACC...`.
Chọn một acc nguồn đã có chuỗi bán; tool copy nguyên các bước sang acc đang chỉnh. Nếu acc đích đã có chuỗi, tool hỏi xác nhận trước khi thay. ClickPoint giữ base size để scale theo cửa sổ acc đích.

REC, copy/paste nhiều dòng, lấy 6 click, route/map/revive, FREEZE/BĐPT và Clean Route v1.5.9 giữ nguyên.
