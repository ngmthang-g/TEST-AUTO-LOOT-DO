# Thần Long NPC / GameDialog Probe v0.1

Probe **chỉ đọc** để chốt runtime của NPC truyền tống như **Xa Truyền Bình / Xa Truyền Công**.

## Probe làm gì

1. Attach vào đúng client Thần Long bằng `WH_GETMESSAGE` giống bridge đã chạy trong source v1.2.
2. `DUMP NPC / object live quanh nhân vật` gọi semantic `GetNearbyObjects()` và liệt kê tối đa những gì runtime expose:
   - dictionary key;
   - Name / Type / ResName;
   - RoleID / ID / NpcID / NPCID / ResID / TemplateID nếu có;
   - Position nếu đọc được.
3. `DUMP GameDialog` chỉ đọc UI đang mở:
   - duyệt cây con của `GUI.FindUI("GameDialog")`;
   - tìm control clickable;
   - đọc Text;
   - đọc `Tag` — với GameDialog chuẩn, `Tag = selectionID`.
4. Mọi kết quả được append vào `NpcDialogProbe_output.txt` cạnh EXE.

## Cách test Xa Truyền

- Mở game và đứng cạnh Xa Truyền Bình hoặc Xa Truyền Công.
- Chạy `ThanLongNpcDialogProbe.exe` cùng quyền với game.
- Chọn đúng PID.
- Chọn mục **2** để dump NPC live quanh nhân vật.
- Chọn mục **4**, sau đó quay sang game và **tự click NPC**. Probe chờ tối đa 15 giây và tự dump GameDialog khi thấy cửa sổ mở.
- Hoặc tự mở NPC trước rồi quay lại chọn mục **3**.
- Gửi lại `NpcDialogProbe_output.txt` để đối chiếu NPC ID và toàn bộ map selection.

## Cam kết read-only của build này

Trong source không có command gameplay mutation: không `ClickNPC`, không `TryClickUI`, không `SendInput`, không AutoPath, không gửi `CMD_SHOW_GAMEDIALOG`, không gọi `HandleClickEvent`.

Bridge chỉ gọi getter/query/read-only và duyệt object/UI runtime.

## Binaries

Luôn để cùng thư mục:

- `ThanLongNpcDialogProbe.exe`
- `ThanLongNpcDialogProbeBridge.dll`

Build: Windows x64 / MSVC / Release.
