# Thần Long Item Consolidator v0.2.3

Windows x64, nền Clean Route v1.5.9. v0.2.3 biến **BỘ ĐIỀU PHỐI TRUNG TÂM (BĐPT)** thành cổng bắt buộc cho mọi click REAL INPUT và tách các chuỗi click khỏi giao diện chính.

## BĐPT là quyền lực trung tâm
Mỗi action vật lý phải đi theo luồng: `request -> BĐPT kiểm quyền -> FREEZE ALL -> cấp lease chuột cho đúng acc -> foreground + SetCursorPos + SendInput -> RESULT OK/FAIL -> UNFREEZE ALL`.

Không account state machine nào được tự gọi REAL INPUT trực tiếp. Raw SendInput chỉ nằm trong routine nội bộ của BĐPT. Khi một click đang được cấp quyền, các account khác chỉ được refresh snapshot, không phát mutable action/click chen ngang.

MAIN và CON đang tham gia một workflow giao dịch có thể bị HOLD xuyên transaction để giữ thứ tự nghiệp vụ; các acc khác vẫn được chạy giữa hai click lease. Vì vậy không còn freeze toàn bộ suốt cả chuỗi như v0.2.2.

## Giao diện gọn theo role
Các bảng tọa độ không nằm phơi trên màn hình chính. Chỉ hiện nút phù hợp với acc đang chọn:

- MAIN: `CHUỖI CLICK BÁN ĐỒ` và `CHUỖI GD MAIN`.
- CON1..CON6: `CHUỖI GD CONx` và `Tọa chọn CON`.
- Role chưa gán: không hiện các nút chuỗi role-specific.

Bấm nút mới mở editor để thêm/xóa/đổi thứ tự/lấy F8/test dòng.

## MAIN trade sequence dùng chung
`CHUỖI GD MAIN` là thư viện bước MAIN dùng chung cho mọi CON. Mỗi bước có tọa độ, delay, repeat và mô tả. Sửa một bước MAIN một lần thì mọi CON tham chiếu bước đó đều dùng cấu hình mới.

## Chuỗi riêng của từng CON
Mỗi CON có `childTradeSequence` riêng, lưu theo profile/RoleID. Mỗi dòng chọn một trong hai loại executor:

- chính CON đó thực hiện click bằng tọa độ riêng của dòng; hoặc
- `MAIN #n`: yêu cầu BĐPT chuyển quyền sang cửa sổ MAIN và chạy bước #n trong `CHUỖI GD MAIN` dùng chung.

`CHUYỂN ĐỒ` chỉ chạy trên CON và vẫn bị giới hạn theo sức chứa còn lại của MAIN.

## Logic túi và ưu tiên giữ nguyên
- MAIN còn `<= 6` ô trống: Auto Sell ưu tiên tuyệt đối, không bắt đầu trade.
- CON chỉ đủ điều kiện khi FULL, `FreeBagSpace == 0`.
- Nhiều CON FULL: cố định `CON1 -> CON2 -> ... -> CON6`, không round-robin.
- MAIN dùng route/recovery v1.5.9 về bãi trước; selected CON về sau.
- Khi hai acc đã sẵn sàng, MAIN click tọa chọn CON rồi BĐPT chạy plan của CON từng bước.

## REAL INPUT
Runtime click dùng foreground đúng cửa sổ + `SetCursorPos` + `SendInput`. Chuột vật lý bị chiếm khi action được cấp lease. Nếu không đưa đúng cửa sổ target lên foreground thì action fail-closed.

CI chỉ chứng minh build/checksum/test logic. Tọa độ UI và delay thật phải test trong game trước với 1 MAIN + 1 CON.
