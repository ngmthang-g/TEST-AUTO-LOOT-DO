# Thần Long NPC / UI Live Probe v0.1.3

Probe **chỉ đọc** để chốt runtime của NPC truyền tống như **Xa Truyền Bình / Xa Truyền Công**.

## Sửa trong v0.1.3

- Sửa enumerator của `GetNearbyObjects()`: IL2CPP generic `Dictionary.Enumerator` và `KeyValuePair` là value-type boxed, nên v0.1.2 gọi getter/MoveNext bằng địa chỉ boxed object làm state không tiến và lặp một key giả. v0.1.3 tự unbox `this` khi invoke method của value-type.
- Lấy `Dictionary.Count` để chặn vòng lặp và phát hiện kết quả bất thường.
- Bỏ phụ thuộc duy nhất vào `GUI.FindUI("GameDialog")`.
- Quét trực tiếp `UIObject.instances` giống cơ chế discovery đã dùng trong source chính: đọc toàn bộ UI live, `Name`, `Text`, `Tag`, `PointerClickHandler`, descendant text và parent path.
- Ưu tiên đánh dấu các nút có text truyền tống như `Đại Lý`, `Lạc Dương`, `Tô Châu`, `Nam Hải`, `Thảo Nguyên`, `Hoàng Long Phủ`, `Miêu Cương`, `Thạch Lâm`...
- Poll mục 4 không còn spam hàng trăm dòng thất bại vào log; chỉ ghi khi bắt được UI đích.

## Cách test

1. Đứng cạnh Xa Truyền Bình hoặc Xa Truyền Công.
2. Chạy EXE cùng quyền với game; chọn bằng STT hoặc PID.
3. Chọn **2** để dump NPC/object live quanh nhân vật.
4. Chọn **4**, quay sang game và tự click NPC. Nếu thấy nhóm nút truyền tống, probe tự ghi UI live.
5. Hoặc giữ bảng NPC đang mở rồi chọn **3**.
6. Gửi lại `NpcDialogProbe_output.txt`.

## Read-only

Không `ClickNPC`, không `TryClickUI`, không `SendInput`, không AutoPath, không gửi selection, không gọi `HandleClickEvent`. Bridge chỉ query/read runtime và UI metadata.

## Binaries

Để cùng thư mục:

- `ThanLongNpcDialogProbe.exe`
- `ThanLongNpcDialogProbeBridge.dll`

Build: Windows x64 / MSVC / Release.
