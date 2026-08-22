from pathlib import Path

root = Path('.')

# protocol bump so EXE/DLL from older probes cannot be mixed accidentally.
p = root / 'src/protocol.h'
s = p.read_text(encoding='utf-8')
s = s.replace('0x00010002', '0x00010003')
p.write_text(s, encoding='utf-8')

# bridge: decode floating boxed Tag values and reject party/faction false positives.
p = root / 'src/bridge.cpp'
s = p.read_text(encoding='utf-8')
if '#include <cmath>' not in s:
    s = s.replace('#include <cstring>\n', '#include <cstring>\n#include <cmath>\n')
old = '''std::wstring ObjText(Il2CppObject* object) {
    if (!object) return L"<null>";
    std::wstring text;
    if (CopyString(object, text)) return text;

    Il2CppClass* klass = gApi.object_get_class(object);
    const char* className = klass ? gApi.class_get_name(klass) : nullptr;
    void* raw = gApi.object_unbox(object);
    if (className && raw) {
        if (std::strcmp(className, "Int32") == 0) return std::to_wstring(*reinterpret_cast<std::int32_t*>(raw));
        if (std::strcmp(className, "UInt32") == 0) return std::to_wstring(*reinterpret_cast<std::uint32_t*>(raw));
        if (std::strcmp(className, "Int64") == 0) return std::to_wstring(*reinterpret_cast<std::int64_t*>(raw));
        if (std::strcmp(className, "UInt64") == 0) return std::to_wstring(*reinterpret_cast<std::uint64_t*>(raw));
        if (std::strcmp(className, "Int16") == 0) return std::to_wstring(*reinterpret_cast<std::int16_t*>(raw));
        if (std::strcmp(className, "UInt16") == 0) return std::to_wstring(*reinterpret_cast<std::uint16_t*>(raw));
        if (std::strcmp(className, "Byte") == 0) return std::to_wstring(*reinterpret_cast<std::uint8_t*>(raw));
        if (std::strcmp(className, "Boolean") == 0) return *reinterpret_cast<std::uint8_t*>(raw) ? L"1" : L"0";
    }
    return className ? L"<" + W(className) + L">" : L"<object>";
}
'''
new = '''std::wstring FloatingText(double value) {
    if (!std::isfinite(value)) {
        std::wostringstream out;
        out << value;
        return out.str();
    }
    const double rounded = std::round(value);
    if (std::fabs(value - rounded) < 1e-9 && std::fabs(rounded) <= 9007199254740991.0) {
        return std::to_wstring(static_cast<long long>(rounded));
    }
    std::wostringstream out;
    out << std::setprecision(15) << value;
    return out.str();
}

std::wstring ObjText(Il2CppObject* object) {
    if (!object) return L"<null>";
    std::wstring text;
    if (CopyString(object, text)) return text;

    Il2CppClass* klass = gApi.object_get_class(object);
    const char* className = klass ? gApi.class_get_name(klass) : nullptr;
    void* raw = gApi.object_unbox(object);
    if (className && raw) {
        if (std::strcmp(className, "Int32") == 0) return std::to_wstring(*reinterpret_cast<std::int32_t*>(raw));
        if (std::strcmp(className, "UInt32") == 0) return std::to_wstring(*reinterpret_cast<std::uint32_t*>(raw));
        if (std::strcmp(className, "Int64") == 0) return std::to_wstring(*reinterpret_cast<std::int64_t*>(raw));
        if (std::strcmp(className, "UInt64") == 0) return std::to_wstring(*reinterpret_cast<std::uint64_t*>(raw));
        if (std::strcmp(className, "Int16") == 0) return std::to_wstring(*reinterpret_cast<std::int16_t*>(raw));
        if (std::strcmp(className, "UInt16") == 0) return std::to_wstring(*reinterpret_cast<std::uint16_t*>(raw));
        if (std::strcmp(className, "SByte") == 0) return std::to_wstring(*reinterpret_cast<std::int8_t*>(raw));
        if (std::strcmp(className, "Byte") == 0) return std::to_wstring(*reinterpret_cast<std::uint8_t*>(raw));
        if (std::strcmp(className, "Boolean") == 0) return *reinterpret_cast<std::uint8_t*>(raw) ? L"1" : L"0";
        if (std::strcmp(className, "Single") == 0) return FloatingText(*reinterpret_cast<float*>(raw));
        if (std::strcmp(className, "Double") == 0) return FloatingText(*reinterpret_cast<double*>(raw));
    }
    return className ? L"<" + W(className) + L">" : L"<object>";
}
'''
if 'std::wstring FloatingText(double value)' not in s:
    if old not in s:
        raise RuntimeError('bridge ObjText anchor not found')
    s = s.replace(old, new)
old = '''    row.path = ParentPath(object);
    row.travel = IsTravelLabel(row.text) || IsTravelLabel(row.name);
    return row;
'''
new = '''    row.path = ParentPath(object);
    const bool travelText = IsTravelLabel(row.text) || IsTravelLabel(row.name);
    const bool obviousNonTravelContext = ContainsAny(
        row.path,
        {L"TeamMemberList", L"MiniTeamFrame", L"RoleHeader", L"TeamRole", L"SpiritHeader"});
    row.travel = travelText && !obviousNonTravelContext;
    return row;
'''
if 'obviousNonTravelContext' not in s:
    if old not in s:
        raise RuntimeError('bridge travel anchor not found')
    s = s.replace(old, new)
p.write_text(s, encoding='utf-8')

# probe: baseline/delta wait instead of stopping on any OK scan.
p = root / 'src/probe.cpp'
s = p.read_text(encoding='utf-8')
if '#include <set>' not in s:
    s = s.replace('#include <string>\n#include <vector>\n', '#include <string>\n#include <vector>\n#include <set>\n#include <sstream>\n')
s = s.replace('v0.1.3', 'v0.1.4')
helpers = r'''struct UiBlock {
    std::wstring key;
    std::wstring body;
    bool travel = false;
};

std::vector<std::wstring> SplitLines(const std::wstring& text) {
    std::vector<std::wstring> lines;
    std::wistringstream input(text);
    std::wstring line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

std::wstring Between(const std::wstring& text, const std::wstring& left, const std::wstring& right) {
    const std::size_t begin = text.find(left);
    if (begin == std::wstring::npos) return L"";
    const std::size_t valueBegin = begin + left.size();
    const std::size_t end = text.find(right, valueBegin);
    if (end == std::wstring::npos) return text.substr(valueBegin);
    return text.substr(valueBegin, end - valueBegin);
}

std::vector<UiBlock> ParseUiBlocks(const std::wstring& text) {
    const auto lines = SplitLines(text);
    std::vector<UiBlock> blocks;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const bool travel = lines[i].rfind(L"[TRAVEL ", 0) == 0;
        const bool ui = lines[i].rfind(L"[UI ", 0) == 0;
        if (!travel && !ui) continue;

        std::wstring parentLine;
        if (i + 1 < lines.size() && lines[i + 1].rfind(L"    parents=", 0) == 0) {
            parentLine = lines[i + 1];
        }

        const std::wstring className = Between(lines[i], L"class=", L" name=");
        const std::wstring name = Between(lines[i], L" name=\"", L"\"");
        std::wstring parents;
        if (!parentLine.empty()) parents = parentLine.substr(std::wstring(L"    parents=").size());

        std::wstring key = className + L"|" + name + L"|" + parents;
        if (name.empty()) {
            const std::size_t textPos = lines[i].find(L" text=");
            key += L"|" + lines[i].substr(0, textPos == std::wstring::npos ? lines[i].size() : textPos);
        }

        UiBlock block;
        block.key = std::move(key);
        block.body = lines[i];
        if (!parentLine.empty()) block.body += L"\n" + parentLine;
        block.travel = travel;
        blocks.push_back(std::move(block));
    }
    return blocks;
}

std::wstring BuildUiDelta(const std::wstring& baseline,
                          const std::wstring& current,
                          std::size_t& newCount,
                          std::size_t& newTravelCount) {
    const auto before = ParseUiBlocks(baseline);
    const auto after = ParseUiBlocks(current);
    std::set<std::wstring> beforeKeys;
    for (const auto& block : before) beforeKeys.insert(block.key);

    std::vector<UiBlock> added;
    for (const auto& block : after) {
        if (beforeKeys.find(block.key) == beforeKeys.end()) added.push_back(block);
    }

    newCount = added.size();
    newTravelCount = 0;
    for (const auto& block : added) if (block.travel) ++newTravelCount;

    std::wostringstream out;
    out << L"=== UI DELTA SAU KHI CLICK NPC — READ ONLY ===\n"
        << L"baselineControls=" << before.size()
        << L" currentControls=" << after.size()
        << L" newControls=" << newCount
        << L" newTravelHits=" << newTravelCount << L"\n\n";

    if (added.empty()) {
        out << L"(Chưa xuất hiện control cấu trúc mới.)\n";
        return out.str();
    }

    std::size_t index = 0;
    for (const auto& block : added) {
        out << L"[NEW " << ++index << L"] " << block.body << L"\n";
    }
    return out.str();
}

'''
if 'struct UiBlock {' not in s:
    s = s.replace('void Menu() {\n', helpers + 'void Menu() {\n')
if 'Đang lấy baseline UI trước khi click NPC' not in s:
    start = s.index('        } else if (choice == 4) {')
    end = s.index('        } else if (choice == 5) {', start)
    newblock = '''        } else if (choice == 4) {
            std::wcout << L"Đang lấy baseline UI trước khi click NPC...\\n";
            if (!session.Send(Command::DumpGameDialog, 5000, false)) {
                std::wcout << L"Không lấy được baseline UI. Thử lại mục 4.\\n";
                continue;
            }
            const std::wstring baseline = session.Text();
            const auto baselineBlocks = ParseUiBlocks(baseline);
            std::wcout << L"Baseline=" << baselineBlocks.size()
                       << L" control. Bây giờ quay sang game và TỰ CLICK Xa Truyền.\\n";

            bool got = false;
            for (int i = 0; i < 30; ++i) {
                Sleep(500);
                if (!session.Send(Command::DumpGameDialog, 5000, false)) continue;

                std::size_t newCount = 0;
                std::size_t newTravelCount = 0;
                const std::wstring delta = BuildUiDelta(
                    baseline, session.Text(), newCount, newTravelCount);

                if (newCount > 0 && newTravelCount > 0 && session.Last() == ResultCode::Ok) {
                    Log(Command::DumpGameDialog, ResultCode::Ok, delta);
                    std::wcout << L"\\n[OK] BẮT ĐƯỢC UI MỚI CỦA XA TRUYỀN:\\n"
                               << delta
                               << L"\\n[Đã ghi NpcDialogProbe_output.txt]\\n";
                    got = true;
                    break;
                }
            }
            if (!got) {
                std::wcout << L"Hết 15 giây chưa thấy UI mới có nút truyền tống. "
                              L"Giữ bảng NPC đang mở rồi chọn mục 3 để dump toàn UI.\\n";
            }
'''
    s = s[:start] + newblock + s[end:]
p.write_text(s, encoding='utf-8')

(root / 'README.md').write_text('''# Thần Long NPC / UI Live Probe v0.1.4

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
''', encoding='utf-8')
