#pragma once
#include <cstdint>
#include <cstdlib>
namespace cleanroute_logic {
struct State { bool valid=false,mapReady=false,waitingMap=false; int mapID=0,x=0,y=0; bool riding=false,autoPathing=false; };
struct Target { int mapID=0,x=0,y=0,tolerance=120; };
enum class Action { Wait, Mount, StartPath, StopPath, Dismount, Hold };
enum class MountAssistAction { Wait, Mount, StartPath, FinishFootCycle };
inline MountAssistAction DecideMountAssist(bool riding,bool autoPathing,int mountAttempts,bool footFallback,std::uint32_t phaseElapsedMs,std::uint32_t mountRetryWaitMs=5000,std::uint32_t footWalkMaxMs=15000){ if(riding)return autoPathing?MountAssistAction::Wait:MountAssistAction::StartPath; if(footFallback){if(phaseElapsedMs>=footWalkMaxMs)return MountAssistAction::FinishFootCycle;return autoPathing?MountAssistAction::Wait:MountAssistAction::StartPath;} if(mountAttempts<=0)return MountAssistAction::Mount; if(mountAttempts==1)return phaseElapsedMs>=mountRetryWaitMs?MountAssistAction::Mount:MountAssistAction::Wait; return phaseElapsedMs>=mountRetryWaitMs?MountAssistAction::StartPath:MountAssistAction::Wait; }
inline bool AtTarget(const State&s,const Target&t){ if(!s.valid||s.mapID!=t.mapID)return false; long long dx=static_cast<long long>(s.x)-t.x,dy=static_cast<long long>(s.y)-t.y,rr=static_cast<long long>(t.tolerance)*t.tolerance; return dx*dx+dy*dy<=rr; }
inline Action Decide(const State&s,const Target&t){ if(!s.valid||!s.mapReady||s.waitingMap)return Action::Wait; if(AtTarget(s,t)){if(s.autoPathing)return Action::StopPath;if(s.riding)return Action::Dismount;return Action::Hold;} if(!s.riding)return Action::Mount;if(!s.autoPathing)return Action::StartPath;return Action::Wait; }
}
