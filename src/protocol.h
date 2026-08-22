#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <cwchar>

namespace thanlong_probe {

constexpr std::uint32_t kMagic = 0x504E544C;
constexpr std::uint32_t kProtocolVersion = 0x00010006;
constexpr UINT kWakeMessage = WM_APP + 0x4D1;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongNpcDialogProbe_";
constexpr std::size_t kDetailChars = 32768;

enum class Command : std::uint32_t {
    None = 0,
    ValidateContext = 1,
    DumpNearbyObjects = 2,
    DumpGameDialog = 3,
    ReadPlayerState = 4,
    ClickTravelSelection = 5,
    ClickMessageBoxConfirm = 6,
};

enum class ResultCode : std::int32_t {
    NotReady = 0,
    Ok = 1,
    BadProtocol = 2,
    WrongThread = 3,
    GameAssemblyMissing = 4,
    Il2CppExportMissing = 5,
    ClassNotFound = 6,
    MethodNotFound = 7,
    InvokeException = 8,
    NullResult = 9,
    EnumerationFailed = 10,
    GameDialogNotOpen = 11,
    FieldReadFailed = 12,
    InternalError = 13,
    SafetyRejected = 14,
    SelectionNotFound = 15,
    ConfirmNotFound = 16,
};

struct SharedBlock {
    std::uint32_t magic = kMagic;
    std::uint32_t protocolVersion = kProtocolVersion;
    std::uint32_t targetPid = 0;
    std::uint32_t targetTid = 0;
    volatile LONG requestSeq = 0;
    volatile LONG completedSeq = 0;
    volatile LONG bridgeBusy = 0;
    Command command = Command::None;
    ResultCode result = ResultCode::NotReady;
    std::int64_t arg0 = 0;
    std::int64_t arg1 = 0;
    std::int64_t out0 = 0;
    std::int64_t out1 = 0;
    wchar_t detail[kDetailChars]{};
};

inline void MappingName(DWORD pid, wchar_t* out, std::size_t cap) {
    if (!out || cap == 0) return;
    swprintf_s(out, cap, L"%s%lu", kMappingPrefix, static_cast<unsigned long>(pid));
}

} // namespace thanlong_probe
