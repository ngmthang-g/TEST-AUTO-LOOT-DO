#include "protocol.h"

#include <windows.h>
#include <tlhelp32.h>
#include <fcntl.h>
#include <io.h>
#include <cstdio>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <sstream>

using namespace thanlong_probe;

namespace {

struct GameClient {
    DWORD pid = 0;
    DWORD tid = 0;
    HWND hwnd = nullptr;
    std::wstring title;
};

std::filesystem::path ExeDir() {
    std::wstring buffer(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    buffer.resize(n);
    return std::filesystem::path(buffer).parent_path();
}

bool HasModule(DWORD pid, const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;

    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool found = false;
    if (Module32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, name) == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return found;
}

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM param) {
    if (!IsWindowVisible(hwnd) || GetWindowTextLengthW(hwnd) <= 0) return TRUE;

    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (!pid || !tid || !HasModule(pid, L"GameAssembly.dll")) return TRUE;

    auto* out = reinterpret_cast<std::vector<GameClient>*>(param);
    for (const auto& g : *out) {
        if (g.pid == pid) return TRUE;
    }

    wchar_t title[512]{};
    GetWindowTextW(hwnd, title, _countof(title));
    out->push_back({pid, tid, hwnd, title});
    return TRUE;
}

std::vector<GameClient> Clients() {
    std::vector<GameClient> out;
    EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&out));
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.pid < b.pid; });
    return out;
}

std::string Utf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string out(static_cast<std::size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), out.data(), n, nullptr, nullptr);
    return out;
}

const wchar_t* ResultText(ResultCode r) {
    switch (r) {
        case ResultCode::Ok: return L"OK";
        case ResultCode::WrongThread: return L"WRONG_THREAD";
        case ResultCode::GameAssemblyMissing: return L"GAMEASSEMBLY_MISSING";
        case ResultCode::Il2CppExportMissing: return L"IL2CPP_EXPORT_MISSING";
        case ResultCode::ClassNotFound: return L"CLASS_NOT_FOUND";
        case ResultCode::MethodNotFound: return L"METHOD_NOT_FOUND";
        case ResultCode::InvokeException: return L"INVOKE_EXCEPTION";
        case ResultCode::NullResult: return L"NULL_RESULT";
        case ResultCode::EnumerationFailed: return L"ENUMERATION_FAILED";
        case ResultCode::GameDialogNotOpen: return L"TRAVEL_UI_NOT_FOUND";
        case ResultCode::FieldReadFailed: return L"FIELD_READ_FAILED";
        case ResultCode::SafetyRejected: return L"SAFETY_REJECTED";
        case ResultCode::SelectionNotFound: return L"SELECTION_NOT_FOUND";
        default: return L"ERROR";
    }
}

const wchar_t* CommandText(Command c) {
    switch (c) {
        case Command::ValidateContext: return L"VALIDATE";
        case Command::DumpNearbyObjects: return L"DUMP_NEARBY";
        case Command::DumpGameDialog: return L"DUMP_GAMEDIALOG";
        case Command::ReadPlayerState: return L"PLAYER";
        case Command::ClickTravelSelection: return L"CALLBACK_TRAVEL";
        default: return L"UNKNOWN";
    }
}

void Log(Command c, ResultCode r, const std::wstring& detail) {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    wchar_t header[256]{};
    _snwprintf_s(
        header,
        _countof(header),
        _TRUNCATE,
        L"\r\n[%04u-%02u-%02u %02u:%02u:%02u] %s => %s\r\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
        CommandText(c), ResultText(r));

    std::ofstream file(ExeDir() / L"NpcDialogProbe_output.txt", std::ios::binary | std::ios::app);
    if (file) file << Utf8(header) << Utf8(detail) << "\r\n";
}

class Session {
public:
    ~Session() { Close(); }

    bool Open(const GameClient& game, std::wstring& error) {
        game_ = game;
        wchar_t mappingName[96]{};
        MappingName(game.pid, mappingName, _countof(mappingName));

        mapping_ = CreateFileMappingW(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
            static_cast<DWORD>(sizeof(SharedBlock)), mappingName);
        if (!mapping_) {
            error = L"CreateFileMapping fail";
            return false;
        }

        shared_ = reinterpret_cast<SharedBlock*>(
            MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
        if (!shared_) {
            error = L"MapViewOfFile fail";
            return false;
        }

        *shared_ = SharedBlock{};
        shared_->targetPid = game.pid;
        shared_->targetTid = game.tid;

        auto dll = ExeDir() / L"ThanLongNpcDialogProbeBridge.dll";
        module_ = LoadLibraryW(dll.c_str());
        if (!module_) {
            error = L"Thiếu/load DLL fail Win32=" + std::to_wstring(GetLastError());
            return false;
        }

        auto proc = reinterpret_cast<HOOKPROC>(GetProcAddress(module_, "TlnpGetMessageHook"));
        if (!proc) {
            error = L"DLL thiếu TlnpGetMessageHook";
            return false;
        }

        hook_ = SetWindowsHookExW(WH_GETMESSAGE, proc, module_, game.tid);
        if (!hook_) {
            error = L"SetWindowsHookEx fail Win32=" + std::to_wstring(GetLastError()) +
                    L" • chạy tool cùng quyền với game";
            return false;
        }

        PostThreadMessageW(game.tid, kWakeMessage, 0, 0);
        return true;
    }

    bool Send(Command command, DWORD timeout = 7000, bool print = true, std::int64_t arg0 = 0) {
        if (!shared_ || !hook_) return false;
        if (shared_->bridgeBusy) {
            if (print) std::wcerr << L"Bridge busy\n";
            return false;
        }

        shared_->command = command;
        shared_->arg0 = arg0;
        shared_->arg1 = 0;
        shared_->result = ResultCode::NotReady;
        shared_->detail[0] = 0;
        LONG seq = shared_->requestSeq + 1;
        MemoryBarrier();
        InterlockedExchange(&shared_->requestSeq, seq);

        if (!PostThreadMessageW(game_.tid, kWakeMessage, 0, 0)) return false;

        ULONGLONG end = GetTickCount64() + timeout;
        while (GetTickCount64() < end) {
            if (shared_->completedSeq == seq) {
                MemoryBarrier();
                last_ = shared_->result;
                text_ = shared_->detail;
                if (print) Log(command, last_, text_);
                if (print) {
                    std::wcout << L"\n[" << ResultText(last_) << L"]\n"
                               << text_ << L"\n[Đã ghi NpcDialogProbe_output.txt]\n";
                }
                return true;
            }
            Sleep(10);
        }

        if (print) std::wcerr << L"TIMEOUT\n";
        return false;
    }

    ResultCode Last() const { return last_; }
    const std::wstring& Text() const { return text_; }

    void Close() {
        if (hook_) UnhookWindowsHookEx(hook_);
        if (module_) FreeLibrary(module_);
        if (shared_) UnmapViewOfFile(shared_);
        if (mapping_) CloseHandle(mapping_);
        hook_ = nullptr;
        module_ = nullptr;
        shared_ = nullptr;
        mapping_ = nullptr;
    }

private:
    GameClient game_{};
    HANDLE mapping_ = nullptr;
    SharedBlock* shared_ = nullptr;
    HMODULE module_ = nullptr;
    HHOOK hook_ = nullptr;
    ResultCode last_ = ResultCode::NotReady;
    std::wstring text_;
};

struct UiBlock {
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

const wchar_t* TravelChoiceText(int choice, std::int64_t& selectionId) {
    selectionId = 0;
    switch (choice) {
        case 1: selectionId = 200001; return L"Đại Lý";
        case 2: selectionId = 200002; return L"Lạc Dương";
        case 3: selectionId = 200003; return L"Tô Châu";
        case 4: selectionId = 200004; return L"Nam Hải";
        case 5: selectionId = 200005; return L"Thảo Nguyên";
        case 6: selectionId = 200006; return L"Hoàng Long Phủ";
        case 7: selectionId = 200007; return L"Miêu Cương";
        case 8: selectionId = 200008; return L"Thạch Lâm";
        case 9: selectionId = 200009; return L"Võ Di";
        case 10: selectionId = 9999; return L"Ta chỉ đi ngang qua (đóng dialog)";
        default: return nullptr;
    }
}

void PrintTravelCallbackMenu() {
    std::wcout
        << L"\n=== TEST CALLBACK XA TRUYỀN — PHẢI TỰ MỞ GAMEDIALOG TRƯỚC ===\n"
        << L" 1. Đại Lý          [200001]\n"
        << L" 2. Lạc Dương       [200002]\n"
        << L" 3. Tô Châu         [200003]\n"
        << L" 4. Nam Hải         [200004]\n"
        << L" 5. Thảo Nguyên     [200005]\n"
        << L" 6. Hoàng Long Phủ  [200006]\n"
        << L" 7. Miêu Cương      [200007]\n"
        << L" 8. Thạch Lâm       [200008]\n"
        << L" 9. Võ Di            [200009]\n"
        << L"10. Ta chỉ đi ngang qua / đóng [9999]\n"
        << L" 0. Hủy\n> ";
}

void Menu() {
    std::wcout
        << L"\n========== THẦN LONG NPC / GAMEDIALOG PROBE v0.1.5 ==========\n"
        << L"Mục 1-5 chỉ đọc. Mục 6 là callback TEST có kiểm soát và SẼ tác động game.\n"
        << L"1. Đọc RoleID / MapID / Pos\n"
        << L"2. DUMP NPC/object live quanh đây\n"
        << L"3. DUMP UI LIVE (Text + Tag + parent path)\n"
        << L"4. CHỜ bảng Xa Truyền 15 giây — rồi tự click NPC\n"
        << L"5. DUMP cả Nearby + UI live\n"
        << L"6. TEST CALLBACK UIButton.HandleClickEvent() của Xa Truyền\n"
        << L"0. Thoát\n> ";
}

void EnableNativeUnicodeConsole() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    std::wcout.clear();
    std::wcerr.clear();
    std::wcin.clear();
}

} // namespace

int wmain() {
    EnableNativeUnicodeConsole();

    std::wcout
        << L"Thần Long NPC / GameDialog Probe v0.1.5 — CONTROLLED CALLBACK TEST\n"
        << L"Mục 1-5 read-only; mục 6 mới có quyền gọi callback Xa Truyền.\n"
        << L"Output: NpcDialogProbe_output.txt\n\n";

    auto games = Clients();
    if (games.empty()) {
        std::wcerr << L"Không tìm thấy client có GameAssembly.dll\n"
                   << L"Nhấn Enter để đóng tool...";
        std::wstring waitLine;
        std::getline(std::wcin, waitLine);
        return 2;
    }

    for (std::size_t i = 0; i < games.size(); ++i) {
        std::wcout << L"  " << (i + 1) << L". " << games[i].title
                   << L" • PID " << games[i].pid << L"\n";
    }

    std::wstring line;
    std::size_t selectedIndex = games.size();
    for (;;) {
        std::wcout << L"Chọn client bằng STT (1-" << games.size() << L") hoặc nhập PID: ";
        std::getline(std::wcin, line);

        unsigned long value = 0;
        try {
            std::size_t used = 0;
            value = std::stoul(line, &used, 10);
            if (used != line.size()) throw std::invalid_argument("extra characters");
        } catch (...) {
            std::wcout << L"Giá trị không hợp lệ. Ví dụ: 1 hoặc PID 13304. Hãy nhập lại.\n";
            continue;
        }

        if (value >= 1 && value <= games.size()) {
            selectedIndex = static_cast<std::size_t>(value - 1);
            break;
        }

        for (std::size_t i = 0; i < games.size(); ++i) {
            if (games[i].pid == static_cast<DWORD>(value)) {
                selectedIndex = i;
                break;
            }
        }

        if (selectedIndex < games.size()) break;
        std::wcout << L"Không có client nào có STT/PID " << value
                   << L". Tool sẽ không tự thoát; hãy nhập lại.\n";
    }

    std::wcout << L"Đã chọn: " << games[selectedIndex].title
               << L" • PID " << games[selectedIndex].pid << L"\n";

    Session session;
    std::wstring error;
    if (!session.Open(games[selectedIndex], error)) {
        std::wcerr << L"Attach fail: " << error << L"\n"
                   << L"Nhấn Enter để đóng tool...";
        std::getline(std::wcin, line);
        return 4;
    }

    session.Send(Command::ValidateContext);

    for (;;) {
        Menu();
        std::getline(std::wcin, line);

        int choice = -1;
        try {
            choice = std::stoi(line);
        } catch (...) {
            continue;
        }

        if (choice == 0) break;
        if (choice == 1) {
            session.Send(Command::ReadPlayerState);
        } else if (choice == 2) {
            session.Send(Command::DumpNearbyObjects);
        } else if (choice == 3) {
            session.Send(Command::DumpGameDialog);
        } else if (choice == 4) {
            std::wcout << L"Đang lấy baseline UI trước khi click NPC...\n";
            if (!session.Send(Command::DumpGameDialog, 5000, false)) {
                std::wcout << L"Không lấy được baseline UI. Thử lại mục 4.\n";
                continue;
            }
            const std::wstring baseline = session.Text();
            const auto baselineBlocks = ParseUiBlocks(baseline);
            std::wcout << L"Baseline=" << baselineBlocks.size()
                       << L" control. Bây giờ quay sang game và TỰ CLICK Xa Truyền.\n";

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
                    std::wcout << L"\n[OK] BẮT ĐƯỢC UI MỚI CỦA XA TRUYỀN:\n"
                               << delta
                               << L"\n[Đã ghi NpcDialogProbe_output.txt]\n";
                    got = true;
                    break;
                }
            }
            if (!got) {
                std::wcout << L"Hết 15 giây chưa thấy UI mới có nút truyền tống. "
                              L"Giữ bảng NPC đang mở rồi chọn mục 3 để dump toàn UI.\n";
            }
        } else if (choice == 5) {
            session.Send(Command::DumpNearbyObjects);
            session.Send(Command::DumpGameDialog);
        } else if (choice == 6) {
            PrintTravelCallbackMenu();
            std::getline(std::wcin, line);
            int travelChoice = -1;
            try { travelChoice = std::stoi(line); } catch (...) { travelChoice = -1; }
            if (travelChoice == 0) {
                std::wcout << L"Đã hủy callback.\n";
                continue;
            }

            std::int64_t selectionId = 0;
            const wchar_t* destination = TravelChoiceText(travelChoice, selectionId);
            if (!destination) {
                std::wcout << L"Lựa chọn không hợp lệ. Không gửi callback.\n";
                continue;
            }

            std::wcout
                << L"Sắp gọi trực tiếp UIButton.HandleClickEvent() cho: " << destination
                << L" • selectionID=" << selectionId << L"\n"
                << L"Tool chỉ cho chạy nếu đang có GameDialog ACTIVE của Xa Truyền và button khớp cả Text + Tag.\n"
                << L"Gõ GO để xác nhận, ký tự khác để hủy: ";
            std::getline(std::wcin, line);
            if (line != L"GO" && line != L"go") {
                std::wcout << L"Đã hủy callback.\n";
                continue;
            }

            if (session.Send(Command::ClickTravelSelection, 7000, true, selectionId) &&
                session.Last() == ResultCode::Ok) {
                std::wcout << L"Callback đã được invoke. Chờ 2.5 giây rồi đọc trạng thái nhân vật...\n";
                Sleep(2500);
                session.Send(Command::ReadPlayerState, 7000, true);
            }
        }
    }

    return 0;
}
