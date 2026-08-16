#pragma once

#include <Windows.h>
#include <cstdint>

namespace remoteloopoc {

constexpr std::uint32_t kMagic = 0x544F4F4C; // "LOOT"
constexpr std::uint32_t kProtocolVersion = 0x00010000;
constexpr UINT kWakeMessage = WM_APP + 0x4A31;
constexpr wchar_t kMappingPrefix[] = L"Local\\RemoteLootPoC_";

enum class Command : std::uint32_t {
    None = 0,
    ValidateContext = 1,
    ResolveLootApi = 2,
    ScanNearestPack = 3,
    ClickObject = 4,
    DirectPickupAll = 5,
    HasCanKhonHoBuff = 6,
    GetFreeBagSpace = 7,
};

enum class ResultCode : std::int32_t {
    Ok = 0,
    NotReady = 1,
    BadProtocol = 2,
    GameAssemblyMissing = 3,
    Il2CppExportMissing = 4,
    NotManagedThread = 5,
    NotUnitySynchronizationContext = 6,
    ClassNotFound = 7,
    MethodNotFound = 8,
    SignatureUnsupported = 9,
    InstanceNotFound = 10,
    InvokeException = 11,
    NoPack = 12,
    FieldReadFailed = 13,
    Timeout = 14,
    InternalError = 15,
};

struct SharedBlock {
    std::uint32_t magic = kMagic;
    std::uint32_t version = kProtocolVersion;
    DWORD controllerPid = 0;
    DWORD targetPid = 0;
    DWORD targetTid = 0;

    volatile LONG requestSequence = 0;
    volatile LONG responseSequence = 0;

    Command command = Command::None;
    std::int64_t arg0 = 0;
    std::int64_t arg1 = 0;
    std::int64_t arg2 = 0;

    ResultCode result = ResultCode::NotReady;
    std::int64_t out0 = 0;
    std::int64_t out1 = 0;
    std::int64_t out2 = 0;
    std::int64_t out3 = 0;

    wchar_t detail[2048]{};
};

} // namespace remoteloopoc
