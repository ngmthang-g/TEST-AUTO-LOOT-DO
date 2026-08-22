# Thần Long NPC / GameDialog Probe v0.1.5

Bản test callback semantic có kiểm soát cho **Xa Truyền Công / Xa Truyền Bình**.

## Đã chốt từ runtime v0.1.4

GameDialog Xa Truyền dùng các UIButton có `Tag/selectionID` ổn định:

- `200001` Đại Lý
- `200002` Lạc Dương
- `200003` Tô Châu
- `200004` Nam Hải
- `200005` Thảo Nguyên
- `200006` Hoàng Long Phủ
- `200007` Miêu Cương
- `200008` Thạch Lâm
- `200009` Võ Di
- `9999` Ta chỉ đi ngang qua / đóng dialog

Tên instance `Button_-xxxxx` là động và **không được dùng làm identity**.

## Mới trong v0.1.5

Mục **6 — TEST CALLBACK** gọi trực tiếp `UIButton.HandleClickEvent()` trên button Xa Truyền sau khi kiểm tra fail-closed:

1. selectionID bắt buộc nằm trong whitelist ở trên;
2. phải có `GameDialog` ACTIVE với `Title` là `Xa Truyền Công`, `Xa Truyền Bình`, `Xa Truyền Chí` hoặc `Xa Truyền Tín`;
3. button phải là `UIButton` ACTIVE bên trong `GameDialog/ButtonList`;
4. button phải khớp **cả Text lẫn Tag/selectionID**;
5. nếu đọc được `Interactable=0` thì từ chối;
6. người dùng phải chọn destination và gõ `GO` trước khi callback.

Không dùng tọa độ, không `TryClickUI`, không `SendInput`, không scroll.

## Cách test callback

1. Đứng cạnh Xa Truyền Công/Bình.
2. **Tự click NPC** để bảng GameDialog truyền tống hiện lên.
3. Chạy tool, chọn đúng client.
4. Chọn mục **6**.
5. Chọn một destination, nên test đầu tiên bằng **Đại Lý [200001]**.
6. Tool in lại Text + selectionID và yêu cầu gõ `GO`.
7. Nếu guard PASS, bridge gọi `UIButton.HandleClickEvent()` trực tiếp.
8. Tool chờ 2.5 giây rồi đọc lại RoleID/MapID/Pos để hỗ trợ xác nhận kết quả.

## Các mục khác

- 1: đọc RoleID / MapID / Pos — read-only.
- 2: dump Nearby objects — read-only.
- 3: dump UI live — read-only.
- 4: baseline + UI delta khi tự click NPC — read-only.
- 5: dump Nearby + UI — read-only.
- 6: **mutation test có kiểm soát** — callback Xa Truyền.

## File cần cùng thư mục

- `ThanLongNpcDialogProbe.exe`
- `ThanLongNpcDialogProbeBridge.dll`

Log: `NpcDialogProbe_output.txt`.
