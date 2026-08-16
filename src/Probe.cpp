#include "Protocol.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace remoteloopoc;

namespace {

struct Target {
    DWORD pid{};
    DWORD tid{};
    std::wstring exe;
};

std::wstring MappingName(DWORD pid) {
    return std::wstring(kMappingPrefix) + std::to_wstring(pid);
}

bool ProcessHasGameAssembly(DWORD pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    bool found = false;
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, L"GameAssembly.dll") == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return found;
}

struct WindowSearch {
    DWORD pid{};
    DWORD tid{};
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* s = reinterpret_cast<WindowSearch*>(lParam);
    DWORD pid = 0;
    const DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (pid != s->pid) return TRUE;
    if (!IsWindow(hwnd)) return TRUE;
    if (GetWindow(hwnd, GW_OWNER) != nullptr) return TRUE;
    s->tid = tid;
    return FALSE;
}

DWORD FindWindowThread(DWORD pid) {
    WindowSearch s{pid, 0};
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&s));
    return s.tid;
}

std::vector<Target> FindTargets() {
    std::vector<Target> out;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return out;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            if (pe.th32ProcessID == GetCurrentProcessId()) continue;
            if (!ProcessHasGameAssembly(pe.th32ProcessID)) continue;
            const DWORD tid = FindWindowThread(pe.th32ProcessID);
            if (!tid) continue;
            out.push_back(Target{pe.th32ProcessID, tid, pe.szExeFile});
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return out;
}

std::filesystem::path ExecutableDirectory() {
    std::wstring buffer(32768, L'\0');
    const DWORD n = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    buffer.resize(n);
    return std::filesystem::path(buffer).parent_path();
}

const wchar_t* ResultName(ResultCode r) {
    switch (r) {
        case ResultCode::Ok: return L"OK";
        case ResultCode::NotReady: return L"NOT_READY";
        case ResultCode::BadProtocol: return L"BAD_PROTOCOL";
        case ResultCode::GameAssemblyMissing: return L"GAMEASSEMBLY_MISSING";
        case ResultCode::Il2CppExportMissing: return L"IL2CPP_EXPORT_MISSING";
        case ResultCode::NotManagedThread: return L"NOT_MANAGED_THREAD";
        case ResultCode::NotUnitySynchronizationContext: return L"NOT_UNITY_SYNCHRONIZATION_CONTEXT";
        case ResultCode::ClassNotFound: return L"CLASS_NOT_FOUND";
        case ResultCode::MethodNotFound: return L"METHOD_NOT_FOUND";
        case ResultCode::SignatureUnsupported: return L"SIGNATURE_UNSUPPORTED";
        case ResultCode::InstanceNotFound: return L"INSTANCE_NOT_FOUND";
        case ResultCode::InvokeException: return L"INVOKE_EXCEPTION";
        case ResultCode::NoPack: return L"NO_PACK";
        case ResultCode::FieldReadFailed: return L"FIELD_READ_FAILED";
        case ResultCode::Timeout: return L"TIMEOUT";
        default: return L"INTERNAL_ERROR";
    }
}

class Session {
public:
    ~Session() { Close(); }

    bool Open(const Target& target) {
        target_ = target;
        const auto mappingName = MappingName(target.pid);
        mapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                      static_cast<DWORD>(sizeof(SharedBlock)), mappingName.c_str());
        if (!mapping_) {
            std::wcerr << L"CreateFileMapping failed: " << GetLastError() << L"\n";
            return false;
        }
        shared_ = static_cast<SharedBlock*>(MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
        if (!shared_) {
            std::wcerr << L"MapViewOfFile failed: " << GetLastError() << L"\n";
            return false;
        }
        *shared_ = SharedBlock{};
        shared_->controllerPid = GetCurrentProcessId();
        shared_->targetPid = target.pid;
        shared_->targetTid = target.tid;

        bridgePath_ = ExecutableDirectory() / L"RemoteLootBridge.dll";
        bridgeModule_ = LoadLibraryW(bridgePath_.c_str());
        if (!bridgeModule_) {
            std::wcerr << L"Cannot load " << bridgePath_.wstring() << L". Win32=" << GetLastError() << L"\n";
            return false;
        }
        auto hookProc = reinterpret_cast<HOOKPROC>(GetProcAddress(bridgeModule_, "RemoteLootGetMessageHook"));
        if (!hookProc) {
            std::wcerr << L"RemoteLootGetMessageHook export missing\n";
            return false;
        }
        hook_ = SetWindowsHookExW(WH_GETMESSAGE, hookProc, bridgeModule_, target.tid);
        if (!hook_) {
            std::wcerr << L"SetWindowsHookEx failed: " << GetLastError()
                       << L". Run the probe at the same privilege level as the game and use the x64 build.\n";
            return false;
        }
        return true;
    }

    bool Send(Command command, std::int64_t a0 = 0, std::int64_t a1 = 0, std::int64_t a2 = 0,
              DWORD timeoutMs = 4000) {
        if (!shared_ || !hook_) return false;
        shared_->command = command;
        shared_->arg0 = a0;
        shared_->arg1 = a1;
        shared_->arg2 = a2;
        shared_->result = ResultCode::NotReady;
        shared_->out0 = shared_->out1 = shared_->out2 = shared_->out3 = 0;
        shared_->detail[0] = L'\0';
        const LONG seq = InterlockedIncrement(&shared_->requestSequence);
        MemoryBarrier();

        if (!PostThreadMessageW(target_.tid, kWakeMessage, 0, 0)) {
            std::wcerr << L"PostThreadMessage failed: " << GetLastError() << L"\n";
            return false;
        }

        const ULONGLONG deadline = GetTickCount64() + timeoutMs;
        while (GetTickCount64() < deadline) {
            if (shared_->responseSequence == seq) {
                MemoryBarrier();
                std::wcout << L"[" << ResultName(shared_->result) << L"] " << shared_->detail << L"\n";
                return true;
            }
            Sleep(10);
        }
        std::wcerr << L"[TIMEOUT] Target thread did not answer within " << timeoutMs << L" ms\n";
        return false;
    }

    SharedBlock* Data() const { return shared_; }

    void Close() {
        if (hook_) {
            UnhookWindowsHookEx(hook_);
            hook_ = nullptr;
        }
        if (bridgeModule_) {
            FreeLibrary(bridgeModule_);
            bridgeModule_ = nullptr;
        }
        if (shared_) {
            UnmapViewOfFile(shared_);
            shared_ = nullptr;
        }
        if (mapping_) {
            CloseHandle(mapping_);
            mapping_ = nullptr;
        }
    }

private:
    Target target_{};
    std::filesystem::path bridgePath_;
    HANDLE mapping_{};
    SharedBlock* shared_{};
    HMODULE bridgeModule_{};
    HHOOK hook_{};
};

std::int64_t ReadId(const wchar_t* prompt, std::int64_t fallback = 0) {
    std::wcout << prompt;
    if (fallback) std::wcout << L" [Enter=" << fallback << L"]";
    std::wcout << L": ";
    std::wstring line;
    std::getline(std::wcin, line);
    if (line.empty()) return fallback;
    try { return std::stoll(line); }
    catch (...) { return 0; }
}

void PrintMenu(std::int64_t lastPack) {
    std::wcout << L"\n=== RemoteLoot PoC v0.1.0 ===\n"
               << L"1. Validate Unity managed context (read-only)\n"
               << L"2. Resolve/print loot API signatures (read-only)\n"
               << L"3. Scan nearest ItemPack (read-only)\n"
               << L"4. TEST ClickToObject(RoleID) WITHOUT MoveTo/MoveToEx\n"
               << L"5. TEST PickUpItemFromItemPack(packID,-1,1) WITHOUT movement\n"
               << L"6. Check Càn Khôn Hồ buff 30008009 (read-only)\n"
               << L"0. Exit\n";
    if (lastPack) std::wcout << L"Last scanned candidate pack RoleID/itemPackID: " << lastPack << L"\n";
    std::wcout << L"> ";
}

} // namespace

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    std::wcout << L"RemoteLoot PoC: one-shot server acceptance probe. No auto-loop, no MoveTo, no MoveToEx.\n";

    auto targets = FindTargets();
    if (targets.empty()) {
        std::wcerr << L"No process with GameAssembly.dll + top-level window was found.\n";
        return 2;
    }

    std::wcout << L"Game candidates:\n";
    for (std::size_t i = 0; i < targets.size(); ++i) {
        std::wcout << L"  " << (i + 1) << L". PID=" << targets[i].pid << L" TID=" << targets[i].tid
                   << L" EXE=" << targets[i].exe << L"\n";
    }
    std::wcout << L"Select [1-" << targets.size() << L"]: ";
    std::wstring line;
    std::getline(std::wcin, line);
    std::size_t selected = 0;
    try { selected = static_cast<std::size_t>(std::stoul(line)); } catch (...) { selected = 0; }
    if (selected < 1 || selected > targets.size()) return 3;

    Session session;
    if (!session.Open(targets[selected - 1])) return 4;

    session.Send(Command::ValidateContext);
    session.Send(Command::ResolveLootApi);

    std::int64_t lastPack = 0;
    for (;;) {
        PrintMenu(lastPack);
        std::getline(std::wcin, line);
        int choice = -1;
        try { choice = std::stoi(line); } catch (...) { choice = -1; }

        if (choice == 0) break;
        if (choice == 1) {
            session.Send(Command::ValidateContext);
        } else if (choice == 2) {
            session.Send(Command::ResolveLootApi);
        } else if (choice == 3) {
            if (session.Send(Command::ScanNearestPack) && session.Data()->result == ResultCode::Ok) {
                lastPack = session.Data()->out0;
            }
        } else if (choice == 4) {
            const auto roleId = ReadId(L"ItemPack RoleID", lastPack);
            if (roleId) {
                std::wcout << L"Do not move the character manually during this single test.\n";
                session.Send(Command::ClickObject, roleId);
            }
        } else if (choice == 5) {
            const auto packId = ReadId(L"itemPackID candidate", lastPack);
            if (packId) {
                std::wcout << L"TEST condition: stand farther than normal pickup range, Auto pickup OFF.\n";
                std::wcout << L"Observe whether the same pack disappears and whether the bag changes.\n";
                session.Send(Command::DirectPickupAll, packId);
            }
        } else if (choice == 6) {
            if (session.Send(Command::HasCanKhonHoBuff) && session.Data()->result == ResultCode::Ok) {
                std::wcout << L"Buff 30008009 = " << (session.Data()->out0 ? L"PRESENT" : L"ABSENT") << L"\n";
            }
        }
    }
    return 0;
}
