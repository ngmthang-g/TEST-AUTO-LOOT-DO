#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace cleanroute {

constexpr std::uint32_t kMagic = 0x4352544Cu; // CRTL
constexpr std::uint32_t kProtocolVersion = 0x00010502u;
constexpr UINT kWakeMessage = WM_APP + 0x531;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongCleanRoute_";

enum class Command : std::uint32_t { None=0, ReadState=1, ToggleRide=2, StartPath=3, StopPath=4, ClickAt=5, ClickNpc=6 };
enum SnapshotValid : std::uint32_t { ValidMapTransition=1u<<0, ValidIdentity=1u<<1, ValidMap=1u<<2, ValidPosition=1u<<3, ValidRiding=1u<<4, ValidMoving=1u<<5, ValidAutoPath=1u<<6, ValidVitals=1u<<7, ValidLifeState=1u<<8, ValidAutoFight=1u<<9, ValidBagSpace=1u<<10, ValidConfirmUi=1u<<11 };

struct Snapshot { std::uint32_t validMask=0, sequence=0; std::int32_t roleID=0,mapID=0,x=0,y=0,riding=0,moving=0,autoPathing=0,mapReady=0,waitingChangeMap=0,hp=0,maxHP=0,dead=0,autoFight=0,freeBagSpace=-1,confirmUiVisible=0; wchar_t characterName[64]{}; };
struct Request { std::uint32_t command=0; std::int32_t arg0=0,arg1=0,arg2=0; };
struct Response { std::int32_t ok=0,errorCode=0; std::uint32_t callbackThreadId=0; Snapshot snapshot{}; wchar_t detail[512]{}; };
struct SharedBlock { std::uint32_t magic=kMagic, protocolVersion=kProtocolVersion, targetPid=0, targetWindowThreadId=0; std::uint64_t targetHwnd=0; volatile LONG requestSeq=0,completedSeq=0,bridgeLoaded=0,bridgeBusy=0; Request request{}; Response response{}; };
inline void MappingName(DWORD pid, wchar_t* output, std::size_t count) { if (!output || count==0) return; wsprintfW(output, L"%s%lu", kMappingPrefix, static_cast<unsigned long>(pid)); }

} // namespace cleanroute
