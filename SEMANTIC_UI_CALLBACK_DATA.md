# THẦN LONG — SEMANTIC UI CALLBACK / RUNTIME UI INTROSPECTION DATA

**Mục tiêu:** tài liệu nền để AI/tool khác có thể nhận diện UI runtime của Thần Long theo vai trò/text/tag/cây cha rồi gọi callback nội bộ, không phụ thuộc tọa độ màn hình.

## 1. Tên cơ chế nên dùng

Tên tổng quát:

- **Runtime UI Introspection**: đọc cây UI/object UI đang tồn tại trong runtime.
- **Semantic UI Resolution**: xác định control theo ý nghĩa (`Text`, `Tag`, `Name`, loại control, parent path, trạng thái) thay vì tọa độ.
- **Direct Managed UI Callback**: gọi trực tiếp method của control, ví dụ `UIButton.HandleClickEvent()`.
- **Semantic UI State Machine**: mỗi bước chỉ hành động sau khi UI/state kế tiếp đã thực sự xuất hiện và được xác minh.

Tên ngắn dùng trong data: **Semantic UI Callback**.

## 2. Kết quả runtime đã xác nhận với Xa Truyền

Khi mở Xa Truyền Công, client sinh top-level `GameDialog` với cấu trúc:

```text
GameDialog
└─ Image_...
   ├─ Title = "Xa Truyền Công"
   └─ ButtonBox : UIScrollView
      └─ ButtonList : UIGridLayout
         ├─ UIButton Text="Đại Lý"        Tag=200001
         ├─ UIButton Text="Lạc Dương"     Tag=200002
         ├─ UIButton Text="Tô Châu"       Tag=200003
         ├─ UIButton Text="Nam Hải"       Tag=200004
         ├─ UIButton Text="Thảo Nguyên"   Tag=200005
         ├─ UIButton Text="Hoàng Long Phủ" Tag=200006
         ├─ UIButton Text="Miêu Cương"    Tag=200007
         ├─ UIButton Text="Thạch Lâm"     Tag=200008
         ├─ UIButton Text="Võ Di"         Tag=200009
         └─ UIButton Text="Ta chỉ đi ngang qua" Tag=9999
```

Tên instance kiểu `Button_-45986`, `Button_-60682`, `Button_-69436` thay đổi mỗi lần mở. Vì vậy **không cache Name/instance pointer làm identity dài hạn**.

Identity bền hơn:

```text
UI root/context + control type + Text + Tag/selectionID + parent path
```

Với Xa Truyền:

```text
GameDialog + UIButton + Text="Đại Lý" + Tag=200001 + parent contains ButtonList
```

Callback `UIButton.HandleClickEvent()` trên button này đã được test runtime thành công.

Sau khi callback destination, game hiện thêm `MessageBox`; cần callback nút xác nhận rồi mới chuyển map.

## 3. Có áp dụng cho mọi NPC không?

### Có thể tái sử dụng framework cho gần như mọi NPC/UI

Nếu mở được NPC thì có thể dùng cùng pipeline để **quan sát UI mới xuất hiện**, dump control và học semantic của nó.

### Nhưng không được kết luận mọi NPC đều dùng đúng một method

Các trường hợp thường gặp:

1. **NPC dùng GameDialog + UIButton**  
   → áp dụng gần như y hệt Xa Truyền: `Text/Tag -> UIButton.HandleClickEvent()`.

2. **NPC mở custom UI**  
   → vẫn quét `UIObject.instances` / `FindUI`, nhưng root có tên khác (`Shop`, `Trade`, `Team`, ...).

3. **Control là UIToggle**  
   → đọc `Selected/Interactable/Text`, thường kích hoạt bằng `set_Selected(true)` thay vì ép `UIButton.HandleClickEvent()`.

4. **Control là UIInput**  
   → dùng `set_Text()` rồi callback nút gửi/xác nhận.

5. **UI chỉ là lớp hiển thị, action thật nằm ở Lua/API**  
   → sau khi nhận diện state có thể gọi action Lua/API trực tiếp nếu đã chứng minh đúng.

6. **Virtualized list**  
   → item ngoài viewport có thể chưa được instantiate; khi đó quét UI object không đảm bảo thấy toàn bộ data. Cần đọc model/dictionary/list backing data hoặc điều khiển scroll semantic.

Vì vậy câu đúng là:

> **Cơ chế Runtime UI Introspection + Semantic Resolver áp dụng rộng cho mọi NPC sau khi mở UI; nhưng callback cuối cùng phải chọn theo loại control/UI thực tế, không mặc định tất cả đều là UIButton.**

## 4. Hai tầng: mở NPC và xử lý UI NPC

### Tầng A — mở NPC

API đã phân tích trong client:

```cpp
FGStudio.LuaSystem.API.LuaSystemAPI_Game.ClickNPC(npcID)
```

Nó là hành vi game nội bộ, không phải chuột Windows. Phân tích client cho thấy chuỗi logic gồm dừng AutoPath, tìm object NPC, quay mặt/chọn target và gửi click-on-object tới game/server.

Nếu NPC live được expose qua dữ liệu gần nhân vật, có thể lấy RoleID/NPC ID bằng read-only API rồi dùng `ClickNPC(id)`.

**Lưu ý:** tên UI NPC và tên live object không nhất thiết giống nhau. Xa Truyền Công trong GameDialog có thể liên quan một live NPC được expose với tên khác như `Dịch Trạm`; vì vậy phải chứng minh mapping bằng runtime, không đoán theo tên.

### Tầng B — xử lý UI sau khi NPC mở

Pipeline chuẩn:

```text
ClickNPC / người dùng mở NPC
        ↓
WAIT: UI mới thực sự xuất hiện
        ↓
DISCOVER: UIObject.instances / MainFindUI / FindUI
        ↓
RESOLVE: class + Text + Tag + parent path + Active + Interactable
        ↓
UNIQUE GUARD: đúng 1 candidate
        ↓
CALLBACK: HandleClickEvent / set_Selected / action phù hợp
        ↓
WAIT: UI/state kế tiếp
        ↓
VERIFY: UI đóng/mở, MapID đổi, state getter đổi, inventory đổi...
        ↓
Bước kế tiếp
```

Không nên viết:

```text
ClickNPC -> Sleep cố định -> callback pointer cũ -> Sleep -> callback pointer cũ khác
```

Vì UI Lua có thể destroy/recreate object sau mỗi bước.

## 5. Các API/hàm quan trọng

### 5.1 IL2CPP metadata/runtime exports

Dùng để resolve class/field/method theo metadata thay vì hardcode pointer:

```text
il2cpp_domain_get
il2cpp_domain_assembly_open
il2cpp_assembly_get_image
il2cpp_class_from_name
il2cpp_class_get_parent
il2cpp_class_get_method_from_name
il2cpp_class_get_field_from_name
il2cpp_field_static_get_value
il2cpp_object_get_class
il2cpp_runtime_invoke
il2cpp_object_unbox
il2cpp_string_chars
il2cpp_string_length
```

### 5.2 UI discovery

Top-level UI khi biết tên:

```cpp
LuaSystemAPI_GUI.MainFindUI("GameDialog")
LuaSystemAPI_GUI.FindUI("GameDialog")
LuaSystemAPI_GUI.MainFindUI("MessageBox")
```

Quét toàn bộ UI khi chưa biết root:

```cpp
FGStudio.LuaSystem.Base.UIObject.instances
```

Mỗi object nên đọc:

```text
ClassName
Name
Text
Tag
ActiveInHierarchy
Interactable
Parent
CoreChildren / descendant text
PointerClickHandler (nếu cần debug)
```

### 5.3 Callback UIButton

```cpp
UIButton.HandleClickEvent()
```

Mẫu gọi IL2CPP:

```cpp
Il2CppClass* klass = il2cpp_object_get_class(button);
MethodInfo* m = ResolveMethod(klass, "HandleClickEvent", 0);
il2cpp_runtime_invoke(m, button, nullptr, &exception);
```

Điều kiện trước callback:

```text
button != null
ActiveInHierarchy == true
Interactable != false
đúng UI context
Text/Tag đúng semantic
candidate duy nhất
```

### 5.4 UIToggle

```text
get_Text()
get_Interactable()
get_Selected()
set_Selected(true/false)
```

Không dùng `UIButton.HandleClickEvent()` mù cho UIToggle.

### 5.5 UIInput

```text
get_Text()
set_Text("...")
```

Sau đó resolve/callback nút Gửi/Xác nhận.

### 5.6 State verify

Ví dụ:

```text
LuaSystemAPI_Game.GetRoleData()
RoleData.MapID / PosX / PosY
LuaSystemAPI_Game.IsMapReady()
SessionData.WaitingChangeMap
GetFreeBagSpace()
get_EnableAutoF1()
LuaLeaderData.IsDeath
```

Action không được coi là thành công chỉ vì `runtime_invoke()` không exception; phải có **state proof** nếu có getter thích hợp.

## 6. RVA đã ghi nhận — chỉ dùng cho đúng client build

Các RVA dưới đây là dữ liệu client-version cụ thể từng được phân tích, **không phải API ổn định giữa mọi bản game**:

| Method | RVA đã ghi nhận |
|---|---:|
| `LuaSystemAPI_Game.ClickNPC(int)` | `0x66ADC0` |
| `LuaSystemAPI_GUI.MainFindUI(string)` | `0x6A5F90` |
| `UIObject.get_Name()` | `0x530240` |
| `UIObject.get_ActiveInHierarchy()` | `0x52F7D0` |
| `UIObject.get_CoreChildren()` | `0x52FB80` |
| `UIButton.get_Interactable()` | `0x52E120` |
| `UIButton.get_Text()` | `0x52E230` |
| `UIButton.HandleClickEvent()` | `0x52D140` |

Khuyến nghị: ưu tiên metadata resolver; RVA chỉ làm signature/reference cho build đã khóa.

## 7. Thuật toán semantic resolver chuẩn

Pseudo-code:

```cpp
UiAction ResolveAction(ActionSpec spec) {
    auto objects = Enumerate(UIObject.instances);
    candidates = [];

    for (obj : objects) {
        if (!obj.ActiveInHierarchy) continue;
        if (!TypeMatches(obj, spec.controlType)) continue;

        row = ReadSemantic(obj); // Name/Text/Tag/ParentPath
        if (!ContextMatches(row.parentPath, spec.uiRoot)) continue;
        if (!TextMatches(row.text, spec.text)) continue;
        if (spec.hasTag && row.tag != spec.tag) continue;
        if (HasInteractable(obj) && !obj.Interactable) continue;

        candidates.push(row);
    }

    if (candidates.size() != 1)
        return FAIL_CLOSED;

    return candidates[0];
}
```

Sau đó:

```cpp
Invoke(action.HandleClickEvent);
WaitForNextSemanticState();
VerifyResult();
```

### Không giữ pointer qua bước UI

Sau mỗi callback:

```text
button A callback
→ game destroy/recreate GameDialog/MessageBox
→ pointer A cũ không còn đáng tin
→ PHẢI enumerate/resolve lại từ đầu cho bước B
```

## 8. Chu trình Xa Truyền hoàn chỉnh

```text
[1] mở Xa Truyền
    ↓
[2] wait GameDialog Title="Xa Truyền ..."
    ↓
[3] resolve destination by Text + Tag + ButtonList parent
    ↓
[4] UIButton.HandleClickEvent(destination)
    ↓
[5] wait MessageBox ACTIVE
    ↓
[6] resolve positive confirm UIButton
    - accept: Xác nhận / Đồng ý / OK / Có
    - reject: Hủy / Không / Đóng / Cancel / No
    - require unique best candidate
    ↓
[7] UIButton.HandleClickEvent(confirm)
    ↓
[8] wait transition evidence
    ↓
[9] verify MapID changed / map ready
```

Quan trọng: không `Sleep()` trong code đang chạy trên **game UI thread**, vì có thể chặn chính UI không cho MessageBox sinh ra. Controller chờ 200 ms rồi gửi **request game-thread mới** để resolve bước kế tiếp.

## 9. Data schema nên lưu cho mỗi NPC

Khuyến nghị lưu JSON/YAML thay vì hardcode C++:

```json
{
  "npc_family": "Xa Truyen",
  "open": {
    "method": "LuaSystemAPI_Game.ClickNPC",
    "npc_id_source": "runtime/live or verified database"
  },
  "ui_root": "GameDialog",
  "title_patterns": ["Xa Truyền Công", "Xa Truyền Bình", "Xa Truyền Chí", "Xa Truyền Tín"],
  "actions": [
    {
      "role": "travel_dai_ly",
      "control_type": "UIButton",
      "text": "Đại Lý",
      "tag": 200001,
      "parent_contains": ["GameDialog", "ButtonList"],
      "invoke": "HandleClickEvent",
      "next_ui": "MessageBox"
    },
    {
      "role": "confirm",
      "control_type": "UIButton",
      "text_positive": ["Xác nhận", "Đồng ý", "OK", "Có"],
      "text_negative": ["Hủy", "Không", "Đóng"],
      "parent_contains": ["MessageBox"],
      "unique": true,
      "invoke": "HandleClickEvent",
      "verify": "MapID changed"
    }
  ]
}
```

Với NPC khác, chỉ cần thay `ui_root/title/actions/verify` sau một lượt probe runtime.

## 10. Quy trình học một NPC mới để đưa vào DATA

1. Đứng cạnh NPC, dump live object nếu API nearby có expose.
2. Baseline toàn UI.
3. Mở NPC thủ công một lần.
4. Diff UI trước/sau.
5. Ghi:
   - root UI mới;
   - title;
   - control type;
   - Text;
   - Tag;
   - parent path;
   - Interactable;
   - state trước/sau.
6. Với từng action, callback thử có kiểm soát.
7. Sau callback, **dump lại từ đầu** để học bước UI tiếp theo.
8. Chỉ khi state verify PASS mới đánh dấu action `RUNTIME_CONFIRMED`.
9. Lưu mapping vào DATA.

Một NPC nhiều bước sẽ thành graph/state-machine:

```text
NPC_OPEN
  -> DIALOG_A/action_1
  -> DIALOG_B/action_2
  -> MESSAGEBOX/confirm
  -> RESULT_STATE
```

## 11. Mức bằng chứng nên gắn trong DATA

- `STATIC_FOUND`: chỉ tìm thấy class/method/string trong client.
- `RUNTIME_OBSERVED`: đã thấy object/UI thật trong log.
- `CALLBACK_INVOKED`: callback trả không exception.
- `RUNTIME_CONFIRMED`: user xác nhận game thực sự thực hiện hành vi mong muốn.
- `VERIFIED_STATE`: có getter/state sau action chứng minh thành công.

Ví dụ Xa Truyền destination hiện đã đạt:

```text
GameDialog structure = RUNTIME_OBSERVED
Text/Tag mapping = RUNTIME_OBSERVED nhiều lần
UIButton callback destination = RUNTIME_CONFIRMED
MessageBox confirm = đang được đưa vào chu trình v0.1.7
```

## 12. Các lỗi cần tránh

1. **Cache `Button_-xxxxx`** — instance name động.
2. **Cache pointer qua UI transition** — object có thể bị destroy/recreate.
3. **Match chỉ bằng Text chung chung** — dễ callback nhầm control ở MainUI.
4. **Không check parent path/UI root** — dễ lấy nút cùng chữ ở UI khác.
5. **Không check Active/Interactable** — có thể gọi control ẩn/disabled.
6. **Có nhiều candidate nhưng vẫn chọn đại** — phải fail-closed.
7. **Dùng timing làm bằng chứng UI tồn tại** — timing chỉ để debounce; UI/state thật mới là evidence.
8. **Block game UI thread bằng Sleep dài** — có thể tự ngăn UI kế tiếp được tạo.
9. **Coi invoke không exception là success gameplay** — cần state verification.
10. **Hardcode RVA cho mọi client version** — metadata/signature phải được kiểm tra lại.

## 13. Kiến trúc khuyến nghị cho auto lớn

```text
Resolver
  ↓
Read-only Snapshot / UI Observer
  ↓
Semantic State Machine
  ↓
Safety Guard + Unique Candidate
  ↓
Serialized Action Queue (mỗi PID chỉ 1 mutable action)
  ↓
Game-window-thread Bridge
  ↓
Direct managed callback / Lua action
  ↓
Fresh Snapshot + Verification
```

Đây là hướng ổn định hơn click tọa độ vì identity là **vai trò của UI**, không phải vị trí pixel.


## Runtime correction v0.1.7 — không hardcode tên container xác nhận

Runtime test v0.1.6 chứng minh callback lựa chọn map PASS nhưng resolver `MessageBox` có thể không thấy popup xác nhận. Quy tắc mới: tên top-level UI chỉ là tín hiệu phụ. Trước action A phải lưu tập UI ACTIVE; sau action A quét lại và ưu tiên control mới ACTIVE theo semantic `Text/descendant Text + Interactable + HandleClickEvent`, đồng thời loại negative actions. `MessageBox/Dialog/Notice/...` chỉ cộng điểm ngữ cảnh, không còn là điều kiện bắt buộc. Nếu không có candidate duy nhất thì fail-closed và dump UI delta.
