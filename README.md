# Thần Long NPC / UI Live Probe v0.1.4

Probe **chỉ đọc** để xác định NPC Xa Truyền và semantic của bảng chọn map.

## Sửa trong v0.1.4

- Mục **4** lấy **baseline UI** trước khi người dùng click NPC, sau đó so sánh cấu trúc control mỗi 500 ms. Không còn dừng chỉ vì scan MainUI trả `OK`.
- Khi bắt được bảng Xa Truyền, log mục 4 chỉ ghi **UI DELTA**: các control mới xuất hiện sau click NPC.
- Loại false-positive tên môn phái/map nằm trong `TeamMemberList`, `MiniTeamFrame`, `RoleHeader`, `SpiritHeader`; ví dụ `Nga My` của thành viên đội không còn bị coi là nút truyền tống.
- `Tag` boxed kiểu `System.Double` / `System.Single` được giải mã thành **giá trị số thật** thay vì `<Double>` / `<Single>`. Nếu GameDialog dùng `Tag = selectionID`, log sẽ hiện ID trực tiếp.
- Giữ sửa lỗi v0.1.3: IL2CPP generic `Dictionary.Enumerator` / `KeyValuePair` value-type được unbox đúng, nên `GetNearbyObjects()` không còn lặp 4096 key giả.

## Cách test

1. Đứng gần **Xa Truyền Công** ở Côn Lôn Sơn hoặc **Xa Truyền Bình** ở Lâu Lan.
2. Chạy EXE cùng quyền với game, chọn client bằng STT hoặc PID.
3. Bấm **4**. Tool chụp baseline UI trước.
4. Quay sang game và **tự click NPC Xa Truyền** trong 15 giây.
5. Giữ bảng chọn map mở. Khi xuất hiện UI mới có `Đại Lý`, `Lạc Dương`, `Tô Châu`..., tool ghi `UI DELTA` vào `NpcDialogProbe_output.txt`.
6. Nếu mục 4 không bắt được, giữ bảng NPC đang mở rồi chọn **3** để dump toàn UI live.

## Read-only

Build này không có command gameplay mutation: không `ClickNPC`, không `TryClickUI`, không `SendInput`, không AutoPath, không gửi selection, không gọi `HandleClickEvent`.

## File cần để cùng thư mục

- `ThanLongNpcDialogProbe.exe`
- `ThanLongNpcDialogProbeBridge.dll`

Output: `NpcDialogProbe_output.txt`.
