# Thần Long Item Consolidator v0.2.8

Windows x64. v0.2.8 làm phiên dồn đồ an toàn hơn và cho phép chuỗi giao dịch lặp theo nhóm nhỏ.

## TỌA ĐỘ GIAO DỊCH chung
- Có nút `TỌA GD` để lấy Map/X/Y từ acc đang đứng tại điểm hẹn do người dùng tự chọn.
- Khi DỒN ĐỒ đang BẬT và một CON đủ điều kiện FULL=0, BĐPT khóa đúng MAIN + CON đó, dừng AutoFight của cả hai và route **cả hai về cùng TỌA ĐỘ GIAO DỊCH** bằng nền route donor v1.5.9.
- Chỉ khi cả hai đã đứng ổn định tại điểm hẹn, không AutoFight/AutoPath/riding và state an toàn thì chuỗi giao dịch mới bắt đầu.
- Nếu MAIN/CON chết, đổi map bất thường hoặc mất state trong lúc chuẩn bị/chuỗi/verify thì phiên dồn abort fail-closed, không click mù.

## Nhóm lặp trong CHUỖI GD ACC CON
- Chọn 2 hoặc nhiều dòng **liên tiếp**, nhập `Lặp nhóm`, bấm `GOM DÒNG ĐÃ CHỌN`.
- Một nhóm có thể gồm cả dòng `ACC CON ĐANG GD` và dòng tham chiếu `MAIN #n`.
- Runtime chạy hết nhóm nhỏ theo đúng thứ tự rồi quay lại đầu nhóm cho tới đủ số vòng, sau đó mới tiếp tục chuỗi lớn.
- `BỎ NHÓM` tháo toàn bộ group của các dòng đang chọn.
- Lặp từng dòng (`Lặp`) vẫn hoạt động bên trong nhóm.
- `CHUYỂN ĐỒ` vẫn chỉ chạy trên CON đang active và mọi click vẫn đi qua BĐPT/REAL INPUT.

## FULL chỉ là cổng vào phiên; dừng theo số ô riêng từng CON
- CON chỉ được **bắt đầu** phiên khi `FreeBagSpace == 0`.
- Mỗi CON có cấu hình riêng `DỪNG GD KHI CON TRỐNG ≥ N ô`.
- Khi CON đã được chọn, BĐPT giữ nguyên `childPid` đó qua nhiều vòng giao dịch; CON không cần FULL lại ở các vòng sau.
- Sau mỗi chuỗi, tool chờ snapshot túi ổn định. Nếu CON chưa đạt N ô trống thì tiếp tục vòng mới với chính CON đó.
- Nếu MAIN chạm ngưỡng bán trong phiên, CON vẫn HOLD; MAIN đi bán bằng workflow hiện có, sau đó quay lại TỌA GD và tiếp tục cùng CON.
- Khi CON đạt `FreeBagSpace >= N`, phiên mới kết thúc và nhả HOLD để core auto-train tiếp tục.

DỒN ĐỒ OFF, auto-train độc lập, sell sequence FREEZE ALL, shared `CHUỖI GD MAIN` + global `CHUỖI GD ACC CON`, REC, copy, 6-click, revive/map/route và BĐPT từ các bản trước được giữ nguyên.
