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
        case ResultCode::GameDialogNotOpen: return L"GAMEDIALOG_NOT_OPEN";
        case ResultCode::FieldReadFailed: return L"FIELD_READ_FAILED";
        default: return L"ERROR";
    }
}

const wchar_t* CommandText(Command c) {
    switch (c) {
        case Command::ValidateContext: return L"VALIDATE";
        case Command::DumpNearbyObjects: return L"DUMP_NEARBY";
        case Command::DumpGameDialog: return L"DUMP_GAMEDIALOG";
        case Command::ReadPlayerState: return L"PLAYER";
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

    bool Send(Command command, DWORD timeout = 7000, bool print = true) {
        if (!shared_ || !hook_) return false;
        if (shared_->bridgeBusy) {
            if (print) std::wcerr << L"Bridge busy\n";
            return false;
        }

        shared_->command = command;
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
                Log(command, last_, text_);
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

void Menu() {
    std::wcout
        << L"\n========== THẦN LONG NPC / GAMEDIALOG PROBE v0.1.1 ==========\n"
        << L"CHỈ ĐỌC: không ClickNPC, TryClick, SendInput, AutoPath, gửi selection.\n"
        << L"1. Đọc RoleID / MapID / Pos\n"
        << L"2. DUMP NPC/object live quanh đây\n"
        << L"3. DUMP GameDialog đang mở (Text + Tag/selectionID)\n"
        << L"4. CHỜ GameDialog 15 giây — chọn rồi quay sang game tự click NPC\n"
        << L"5. DUMP cả Nearby + GameDialog\n"
        << L"0. Thoát\n> ";
}

void EnableNativeUnicodeConsole() {
    // MSVC wide streams can fail at the first Vietnamese character when stdout
    // remains in the default text mode. Example: "Thần" becomes only "Th".
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
        << L"Thần Long NPC / GameDialog Probe v0.1.1 — READ ONLY\n"
        << L"Output: NpcDialogProbe_output.txt\n\n";

    auto games = Clients();
    if (games.empty()) {
        std::wcerr << L"Không tìm thấy client có GameAssembly.dll\n";
        return 2;
    }

    for (std::size_t i = 0; i < games.size(); ++i) {
        std::wcout << L"  " << (i + 1) << L". " << games[i].title
                   << L" • PID " << games[i].pid << L"\n";
    }

    std::wcout << L"Chọn client: ";
    std::wstring line;
    std::getline(std::wcin, line);

    std::size_t index = 0;
    try {
        index = std::stoul(line);
    } catch (...) {
        return 3;
    }
    if (index < 1 || index > games.size()) return 3;

    Session session;
    std::wstring error;
    if (!session.Open(games[index - 1], error)) {
        std::wcerr << L"Attach fail: " << error << L"\n";
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
            std::wcout << L"Đang chờ. Quay sang game và TỰ CLICK Xa Truyền...\n";
            bool got = false;
            for (int i = 0; i < 30; ++i) {
                if (session.Send(Command::DumpGameDialog, 5000, false) &&
                    session.Last() == ResultCode::Ok) {
                    std::wcout << L"\n[OK] BẮT ĐƯỢC GAMEDIALOG:\n"
                               << session.Text()
                               << L"\n[Đã ghi NpcDialogProbe_output.txt]\n";
                    got = true;
                    break;
                }
                Sleep(500);
            }
            if (!got) std::wcout << L"Hết 15 giây chưa bắt được GameDialog.\n";
        } else if (choice == 5) {
            session.Send(Command::DumpNearbyObjects);
            session.Send(Command::DumpGameDialog);
        }
    }

    return 0;
}
