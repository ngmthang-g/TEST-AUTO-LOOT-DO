# Thần Long Item Consolidator v0.2.9

v0.2.9 sửa đúng luồng giao dịch thực tế sau khi MAIN + CON về điểm hẹn: mở panel tổ đội trước, khóa cứng hai acc tại TỌA GD suốt phiên, và làm F4 pause có đường fallback.

## 1. TỌA TỔ ĐỘI
- Chọn một CON1..CON6, bấm `TỌA TỔ ĐỘI`, đưa chuột vào nút mở danh sách tổ đội trên cửa sổ MAIN rồi nhấn F8.
- Điểm này là global trên MAIN, dùng chung cho mọi CON.
- Mỗi vòng giao dịch: MAIN + CON phải GD LOCK tại TỌA GD -> MAIN click TỌA TỔ ĐỘI -> chờ panel -> MAIN click tọa mặt CONn riêng -> chạy `CHUỖI GD ACC CON`.

## 2. GD LOCK: cấm AutoPath train/map trong phiên
- Trade rendezvous dùng `tradeRendezvousPhase` riêng, không dùng `trainRecoveryPhase`.
- MAIN + active CON vẫn `tradeHeld`, nên `TickAccount` train/map không được chạy.
- Tới TỌA GD: tool gửi StopPath, verify AutoPath=OFF và đứng trong tolerance rồi mới click.
- Trong OpenParty / SelectChild / Sequence / Verify: nếu AutoPath tự bật lại, tool gửi StopPath và không cấp click tiếp. Nếu nhân vật thực sự rời TỌA GD thì abort fail-closed.
- Khi MAIN bán giữa phiên, CON tiếp tục GD LOCK tại TỌA GD. MAIN bán xong không chạy sell phase quay bãi; coordinator đưa MAIN thẳng về TỌA GD rồi tiếp tục đúng childPid đã pin.
- Chỉ FinishTrade/AbortTrade mới nhả hold để core auto-train/map route hoạt động lại.

## 3. F4 PAUSE
- Vẫn dùng `RegisterHotKey(F4)` khi Windows cho phép.
- Đồng thời poll cạnh phím bằng `GetAsyncKeyState(VK_F4)` + debounce 350 ms. Nếu global registration bị app/game khác chiếm, fallback vẫn có thể đổi PAUSE/RESUME.
- Khi PAUSE: scheduler account và trade coordinator đều dừng cấp action; StopPath gửi cho toàn bộ acc RUN; transaction state giữ nguyên để Resume tiếp đúng phiên.

## Các rule v0.2.8 giữ nguyên
- CON chỉ bắt đầu phiên khi `FreeBagSpace == 0`.
- Khi đã chọn CON, childPid giữ nguyên qua nhiều vòng tới khi đạt số ô trống riêng.
- Nhóm lặp nhiều dòng + row repeat giữ nguyên.
- MAIN <= ngưỡng bán vẫn ưu tiên sell; shared MAIN workflow + một global ACC CON workflow; fixed CON1->CON6; BĐPT REAL INPUT; đúng 2 SendInput.
