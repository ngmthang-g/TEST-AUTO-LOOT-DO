from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def replace_once(path, old, new):
    p = ROOT / path
    s = p.read_text(encoding='utf-8')
    if old not in s:
        raise SystemExit(f'pattern not found in {path}: {old[:80]!r}')
    p.write_text(s.replace(old, new, 1), encoding='utf-8')

def replace_all(path, old, new):
    p = ROOT / path
    s = p.read_text(encoding='utf-8')
    if old not in s:
        raise SystemExit(f'pattern not found in {path}: {old!r}')
    p.write_text(s.replace(old, new), encoding='utf-8')

replace_once('src/protocol.h', '0x00010005', '0x00010006')

replace_once('src/bridge.cpp',
    'SharedBlock* gShared = nullptr;\n',
    'SharedBlock* gShared = nullptr;\nstd::vector<Il2CppObject*> gTravelUiBaselineActive;\n')

replace_once('src/bridge.cpp', r'''int PositiveConfirmScore(const std::wstring& label) {
    const std::wstring folded = FoldSemanticLabel(label);
    if (folded == L"xacnhan") return 100;
    if (folded == L"dongy" || folded == L"chapnhan") return 95;
    if (folded == L"confirm") return 90;
    if (folded == L"ok") return 85;
    if (folded == L"co" || folded == L"yes") return 70;
    return 0;
}
''', r'''int PositiveConfirmScore(const std::wstring& label) {
    const std::wstring folded = FoldSemanticLabel(label);
    if (folded == L"xacnhan") return 100;
    if (folded.find(L"xacnhan") != std::wstring::npos) return 98;
    if (folded == L"dongy" || folded == L"chapnhan") return 95;
    if (folded.find(L"dongy") != std::wstring::npos || folded.find(L"chapnhan") != std::wstring::npos) return 93;
    if (folded == L"confirm") return 90;
    if (folded.find(L"confirm") != std::wstring::npos) return 88;
    if (folded == L"ok") return 85;
    if (folded == L"co" || folded == L"yes") return 70;
    return 0;
}
''')

replace_once('src/bridge.cpp', r'''    Il2CppClass* klass = gApi.object_get_class(target.object);
    const MethodInfo* handleClick = Method(klass, "HandleClickEvent", 0);
''', r'''    // Capture the ACTIVE UI set immediately before the destination callback.
    // The confirmation popup may use any runtime container name, so the next stage
    // resolves newly-active semantic controls instead of assuming "MessageBox".
    gTravelUiBaselineActive.clear();
    for (Il2CppObject* object : objects) {
        if (object && IsActiveUiObject(object)) gTravelUiBaselineActive.push_back(object);
    }

    Il2CppClass* klass = gApi.object_get_class(target.object);
    const MethodInfo* handleClick = Method(klass, "HandleClickEvent", 0);
''')

p = ROOT / 'src/bridge.cpp'
s = p.read_text(encoding='utf-8')
start = s.index('ResultCode ClickMessageBoxConfirm(std::wstring& detail) {')
end = s.index('\nbool EnsureShared()', start)
new_func = r'''ResultCode ClickMessageBoxConfirm(std::wstring& detail) {
    std::vector<Il2CppObject*> objects;
    std::int32_t dictionaryCount = 0;
    std::uintptr_t capacity = 0;
    std::wstring why;
    if (!EnumerateUiObjects(objects, dictionaryCount, capacity, why)) {
        detail = L"Không enumerate được UIObject.instances: " + why;
        return ResultCode::EnumerationFailed;
    }

    struct Candidate {
        UiRow row;
        int score = 0;
        bool newlyActive = false;
        bool dialogContext = false;
    };
    std::vector<Candidate> candidates;
    std::vector<UiRow> newlyActiveRows;
    std::size_t activeCount = 0;

    const auto wasActiveBeforeTravel = [](Il2CppObject* object) {
        return std::find(gTravelUiBaselineActive.begin(), gTravelUiBaselineActive.end(), object) !=
               gTravelUiBaselineActive.end();
    };

    for (Il2CppObject* object : objects) {
        if (!object || !IsActiveUiObject(object)) continue;
        ++activeCount;

        UiRow row = ReadUiRow(object);
        const bool newlyActive = !wasActiveBeforeTravel(object);
        const bool dialogContext = ContainsAny(
            row.path + L"/" + row.name,
            {L"MessageBox", L"Dialog", L"Confirm", L"Notice", L"Prompt", L"Tip", L"Warning", L"Alert", L"Ask"});

        if (newlyActive) {
            const bool useful = row.clickable || !row.text.empty() || !row.tag.empty() ||
                                ContainsAny(row.name, {L"Button", L"Dialog", L"Box", L"Notice", L"Confirm"});
            if (useful && newlyActiveRows.size() < 100) newlyActiveRows.push_back(row);
        }

        if (!row.clickable) continue;
        Il2CppClass* klass = gApi.object_get_class(row.object);
        if (!klass || !Method(klass, "HandleClickEvent", 0)) continue;
        if (IsNegativeConfirmLabel(row.text) || IsNegativeConfirmLabel(row.name)) continue;

        double interactable = 1.0;
        bool integral = true;
        if (NumberMember(row.object, "Interactable", interactable, integral) && interactable < 0.5) continue;

        int semanticScore = std::max(PositiveConfirmScore(row.text), PositiveConfirmScore(row.name));
        if (semanticScore <= 0) continue;

        int score = semanticScore;
        if (newlyActive) score += 40;
        if (dialogContext) score += 20;
        candidates.push_back({std::move(row), score, newlyActive, dialogContext});
    }

    if (candidates.empty()) {
        std::wostringstream out;
        out << L"CONFIRM_NOT_FOUND: chưa thấy control positive có HandleClickEvent sau callback điểm đến."
            << L"\nBaselineActive=" << gTravelUiBaselineActive.size()
            << L" currentActive=" << activeCount
            << L" newlyActiveUseful=" << newlyActiveRows.size()
            << L"\nKhông còn bắt buộc container tên MessageBox. Dưới đây là UI mới đang ACTIVE:";
        if (newlyActiveRows.empty()) {
            out << L"\n(không có control mới hữu ích; popup có thể tái sử dụng object cũ — chọn mục 3 để dump toàn UI)";
        } else {
            for (std::size_t i = 0; i < newlyActiveRows.size(); ++i) {
                out << L"\n";
                EmitUiRow(out, newlyActiveRows[i], i + 1, L"NEW-CONFIRM");
            }
        }
        detail = out.str();
        return ResultCode::ConfirmNotFound;
    }

    int bestScore = 0;
    for (const auto& c : candidates) bestScore = std::max(bestScore, c.score);
    std::vector<Candidate*> best;
    for (auto& c : candidates) if (c.score == bestScore) best.push_back(&c);

    if (best.size() != 1 || !best.front() || !best.front()->row.object) {
        std::wostringstream out;
        out << L"SAFETY REJECT: có " << best.size()
            << L" control cùng điểm semantic confirm=" << bestScore << L"; không callback mù.";
        for (const auto* c : best) {
            if (!c) continue;
            out << L"\n- class=" << c->row.className
                << L" text=\"" << c->row.text << L"\" name=\"" << c->row.name << L"\""
                << L" new=" << (c->newlyActive ? 1 : 0)
                << L" dialogContext=" << (c->dialogContext ? 1 : 0)
                << L" parents=" << c->row.path;
        }
        detail = out.str();
        return ResultCode::SafetyRejected;
    }

    Candidate& chosen = *best.front();
    UiRow& target = chosen.row;
    Il2CppClass* klass = gApi.object_get_class(target.object);
    const MethodInfo* handleClick = Method(klass, "HandleClickEvent", 0);
    if (!handleClick) {
        detail = L"Control Xác nhận không resolve được HandleClickEvent().";
        return ResultCode::MethodNotFound;
    }

    Il2CppObject* ignored = nullptr;
    if (!Invoke(handleClick, ManagedThis(target.object), nullptr, ignored)) {
        detail = L"Managed exception khi callback control Xác nhận.";
        return ResultCode::InvokeException;
    }

    std::wostringstream out;
    out << L"CONFIRM CALLBACK ĐÃ GỌI"
        << L" • class=" << target.className
        << L" • text=\"" << target.text << L"\""
        << L" • score=" << bestScore
        << L" • newlyActive=" << (chosen.newlyActive ? 1 : 0)
        << L" • dialogContext=" << (chosen.dialogContext ? 1 : 0);
    if (!target.name.empty()) out << L" • button=" << target.name;
    if (!target.path.empty()) out << L"\nparents=" << target.path;
    out << L"\nĐã invoke trực tiếp HandleClickEvent() theo UI-delta + semantic label; không tọa độ/TryClick/SendInput.";
    detail = out.str();
    gTravelUiBaselineActive.clear();
    return ResultCode::Ok;
}
'''
p.write_text(s[:start] + new_func + s[end:], encoding='utf-8')
replace_all('src/bridge.cpp', 'v0.1.6 có chu trình mutation có kiểm soát', 'v0.1.7 có chu trình mutation có kiểm soát')

replace_all('src/probe.cpp', 'v0.1.6', 'v0.1.7')
replace_once('src/probe.cpp', 'Mục 1-5 chỉ đọc. Mục 6 chạy chu trình callback Xa Truyền → MessageBox Xác nhận', 'Mục 1-5 chỉ đọc. Mục 6 chạy chu trình callback Xa Truyền → UI Xác nhận semantic')
replace_once('src/probe.cpp', 'Mục 1-5 read-only; mục 6 callback lựa chọn Xa Truyền rồi callback MessageBox Xác nhận.', 'Mục 1-5 read-only; mục 6 callback lựa chọn Xa Truyền rồi resolve/callback UI Xác nhận semantic.')
replace_once('src/probe.cpp', 'Đã callback điểm đến. Đang chờ MessageBox Xác nhận xuất hiện...', 'Đã callback điểm đến. Đang chờ UI xác nhận semantic xuất hiện...')
replace_once('src/probe.cpp', 'for (int i = 0; i < 30; ++i) {\n                Sleep(200);\n                if (!session.Send(Command::ClickMessageBoxConfirm, 5000, false)) continue;', 'for (int i = 0; i < 40; ++i) {\n                Sleep(200);\n                if (!session.Send(Command::ClickMessageBoxConfirm, 5000, false)) continue;')
replace_once('src/probe.cpp', r'''            if (!confirmed) {
                std::wcout << L"Không callback được Xác nhận trong 6 giây. Giữ popup đang mở rồi chọn mục 3 để dump UI.\n";
                continue;
            }
''', r'''            if (!confirmed) {
                Log(Command::ClickMessageBoxConfirm, session.Last(), session.Text());
                std::wcout << L"Không callback được Xác nhận trong 8 giây. Resolver v0.1.7 đã dump UI mới bên dưới:\n"
                           << session.Text() << L"\n[Đã ghi NpcDialogProbe_output.txt]\n";
                continue;
            }
''')

replace_all('README.md', 'v0.1.6', 'v0.1.7')
with (ROOT / 'README.md').open('a', encoding='utf-8') as f:
    f.write('''\n\n## v0.1.7 — Confirm resolver theo UI delta\n\n- Không còn bắt buộc popup xác nhận phải có container tên `MessageBox`.\n- Trước callback điểm đến, Bridge lưu tập UI đang ACTIVE làm baseline.\n- Sau callback, resolver ưu tiên control mới ACTIVE, đọc `Text` trực tiếp hoặc text cây con, kiểm tra `Interactable`, loại semantic âm (`Hủy/Không/Đóng/...`) và chỉ gọi đúng một control positive.\n- Matcher chấp nhận cả nhãn chứa `Xác nhận`, `Đồng ý`, `Chấp nhận`, `Confirm`, không chỉ khớp tuyệt đối.\n- Nếu không tìm thấy, log tự dump tối đa 100 control mới để khóa chính xác popup runtime.\n''')
replace_all('SEMANTIC_UI_CALLBACK_DATA.md', 'v0.1.6', 'v0.1.7')
with (ROOT / 'SEMANTIC_UI_CALLBACK_DATA.md').open('a', encoding='utf-8') as f:
    f.write('''\n\n## Runtime correction v0.1.7 — không hardcode tên container xác nhận\n\nRuntime test v0.1.6 chứng minh callback lựa chọn map PASS nhưng resolver `MessageBox` có thể không thấy popup xác nhận. Quy tắc mới: tên top-level UI chỉ là tín hiệu phụ. Trước action A phải lưu tập UI ACTIVE; sau action A quét lại và ưu tiên control mới ACTIVE theo semantic `Text/descendant Text + Interactable + HandleClickEvent`, đồng thời loại negative actions. `MessageBox/Dialog/Notice/...` chỉ cộng điểm ngữ cảnh, không còn là điều kiện bắt buộc. Nếu không có candidate duy nhất thì fail-closed và dump UI delta.\n''')

print('Applied ThanLongNpcDialogProbe v0.1.7 UI-delta confirm resolver')
