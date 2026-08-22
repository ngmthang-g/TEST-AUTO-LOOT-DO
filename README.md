# Thần Long NPC / GameDialog Probe v0.1.6

Bản test chu trình semantic callback có kiểm soát cho **Xa Truyền Công / Xa Truyền Bình / Xa Truyền Chí / Xa Truyền Tín**.

## Runtime đã chốt

GameDialog Xa Truyền dùng các `UIButton` có `Tag/selectionID` ổn định:

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

## Mới trong v0.1.6

Mục **6 — TEST CHU TRÌNH Xa Truyền** chạy:

`GameDialog Xa Truyền -> callback destination -> chờ MessageBox -> callback Xác nhận -> chờ MapID đổi`.

### Guard destination

1. selectionID bắt buộc nằm trong whitelist ở trên;
2. phải có `GameDialog` ACTIVE với `Title` thuộc nhóm Xa Truyền;
3. button phải là `UIButton` ACTIVE bên trong `GameDialog/ButtonList`;
4. button phải khớp **cả Text + Tag/selectionID**;
5. nếu `Interactable=0` thì từ chối;
6. người dùng phải gõ `GO`.

### Guard Xác nhận

Sau callback destination, controller tạo request mới sau mỗi 200 ms để UI thread có thời gian sinh popup. Bridge chỉ xét `UIButton` ACTIVE + interactable có parent path chứa `MessageBox`, loại nhãn âm như Hủy/Không/Đóng và chỉ nhận nhãn dương như `Xác nhận`, `Đồng ý`, `OK`, `Có`. Nếu có nhiều candidate đồng hạng thì fail-closed.

Không block game UI thread bằng Sleep trong Bridge; toàn bộ chờ/poll nằm ở controller và mỗi callback là một request game-thread riêng.

Sau callback Xác nhận, tool poll `ReadPlayerState` tối đa 15 giây và xác minh `MapID` thay đổi.

Không dùng tọa độ, không `TryClickUI`, không `SendInput`, không scroll.

## Cách test

1. Đứng cạnh Xa Truyền.
2. **Tự mở GameDialog Xa Truyền**.
3. Chạy tool, chọn đúng client.
4. Chọn mục **6**.
5. Chọn destination, ví dụ **Đại Lý [200001]**.
6. Gõ `GO`.
7. Tool tự callback destination, tự chờ/callback MessageBox Xác nhận, rồi theo dõi MapID.

## Các mục khác

- 1: đọc RoleID / MapID / Pos — read-only.
- 2: dump Nearby objects — read-only.
- 3: dump UI live — read-only.
- 4: baseline + UI delta khi tự click NPC — read-only.
- 5: dump Nearby + UI — read-only.
- 6: **mutation test có kiểm soát** — chọn map + Xác nhận.

## Tài liệu data

Xem `SEMANTIC_UI_CALLBACK_DATA.md` để tái sử dụng cơ chế cho các NPC/UI khác.

## File cần cùng thư mục

- `ThanLongNpcDialogProbe.exe`
- `ThanLongNpcDialogProbeBridge.dll`

Log: `NpcDialogProbe_output.txt`.
