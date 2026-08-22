#include "protocol.h"

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace thanlong_probe;

namespace {
using Il2CppDomain = void;
using Il2CppAssembly = void;
using Il2CppImage = void;
using Il2CppClass = void;
using Il2CppObject = void;
using Il2CppString = void;
using Il2CppType = void;
using MethodInfo = void;
using FieldInfo = void;

HANDLE gMapping = nullptr;
SharedBlock* gShared = nullptr;

std::wstring W(const char* s) {
    if (!s || !*s) return L"";
    const int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (n <= 1) return L"";
    std::wstring out(static_cast<std::size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, out.data(), n);
    out.resize(static_cast<std::size_t>(n - 1));
    return out;
}

template <typename T> bool Resolve(HMODULE m, const char* n, T& out) {
    out = reinterpret_cast<T>(GetProcAddress(m, n));
    return out != nullptr;
}

struct Api {
    HMODULE module = nullptr;
    Il2CppDomain* (__cdecl* domain_get)() = nullptr;
    const Il2CppAssembly* (__cdecl* domain_assembly_open)(Il2CppDomain*, const char*) = nullptr;
    const Il2CppImage* (__cdecl* assembly_get_image)(const Il2CppAssembly*) = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    const char* (__cdecl* class_get_name)(Il2CppClass*) = nullptr;
    const char* (__cdecl* class_get_namespace)(Il2CppClass*) = nullptr;
    std::size_t (__cdecl* image_get_class_count)(const Il2CppImage*) = nullptr;
    Il2CppClass* (__cdecl* image_get_class)(const Il2CppImage*, std::size_t) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    const Il2CppType* (__cdecl* field_get_type)(FieldInfo*) = nullptr;
    void (__cdecl* field_get_value)(Il2CppObject*, FieldInfo*, void*) = nullptr;
    void (__cdecl* field_static_get_value)(FieldInfo*, void*) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    const Il2CppType* (__cdecl* method_get_return_type)(const MethodInfo*) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppString*) = nullptr;
    const wchar_t* (__cdecl* string_chars)(Il2CppString*) = nullptr;
    Il2CppString* (__cdecl* string_new)(const char*) = nullptr;

    bool Load(std::wstring& why) {
        if (module) return true;
        module = GetModuleHandleW(L"GameAssembly.dll");
        if (!module) { why = L"GameAssembly.dll chưa sẵn sàng"; return false; }
#define NEED(f,n) do { if (!Resolve(module,n,f)) { why = L"Thiếu IL2CPP export: " L##n; return false; } } while(0)
        NEED(domain_get,"il2cpp_domain_get");
        NEED(domain_assembly_open,"il2cpp_domain_assembly_open");
        NEED(assembly_get_image,"il2cpp_assembly_get_image");
        NEED(class_from_name,"il2cpp_class_from_name");
        NEED(class_get_parent,"il2cpp_class_get_parent");
        NEED(class_get_method_from_name,"il2cpp_class_get_method_from_name");
        NEED(class_get_name,"il2cpp_class_get_name");
        NEED(class_get_namespace,"il2cpp_class_get_namespace");
        NEED(image_get_class_count,"il2cpp_image_get_class_count");
        NEED(image_get_class,"il2cpp_image_get_class");
        NEED(class_get_field_from_name,"il2cpp_class_get_field_from_name");
        NEED(field_get_type,"il2cpp_field_get_type");
        NEED(field_get_value,"il2cpp_field_get_value");
        NEED(field_static_get_value,"il2cpp_field_static_get_value");
        NEED(method_get_flags,"il2cpp_method_get_flags");
        NEED(method_get_return_type,"il2cpp_method_get_return_type");
        NEED(type_get_name,"il2cpp_type_get_name");
        NEED(free_fn,"il2cpp_free");
        NEED(runtime_invoke,"il2cpp_runtime_invoke");
        NEED(object_unbox,"il2cpp_object_unbox");
        NEED(object_get_class,"il2cpp_object_get_class");
        NEED(string_length,"il2cpp_string_length");
        NEED(string_chars,"il2cpp_string_chars");
        NEED(string_new,"il2cpp_string_new");
#undef NEED
        return true;
    }
} gApi;

const Il2CppImage* Image() {
    auto* d = gApi.domain_get ? gApi.domain_get() : nullptr;
    if (!d) return nullptr;
    const Il2CppAssembly* a = gApi.domain_assembly_open(d,"Assembly-CSharp");
    if (!a) a = gApi.domain_assembly_open(d,"Assembly-CSharp.dll");
    return a ? gApi.assembly_get_image(a) : nullptr;
}

bool IsStatic(const MethodInfo* m) {
    if (!m) return false;
    std::uint32_t f=0,i=0;
    return (gApi.method_get_flags(m,&i) & 0x0010u) != 0;
}

const MethodInfo* Method(Il2CppClass* c, const char* n, int argc) {
    for (; c; c=gApi.class_get_parent(c)) if (const MethodInfo* m=gApi.class_get_method_from_name(c,n,argc)) return m;
    return nullptr;
}

Il2CppClass* Class(const char* ns, const char* name) {
    const Il2CppImage* img=Image(); if(!img) return nullptr;
    if (Il2CppClass* c=gApi.class_from_name(img,ns,name)) return c;
    const auto count=gApi.image_get_class_count(img);
    for(std::size_t i=0;i<count && i<65536;i++){
        Il2CppClass* c=gApi.image_get_class(img,i);
        const char* n=c?gApi.class_get_name(c):nullptr;
        if(n && std::strcmp(n,name)==0) return c;
    }
    return nullptr;
}

FieldInfo* Field(Il2CppClass* c,const char* n){ for(;c;c=gApi.class_get_parent(c)) if(auto* f=gApi.class_get_field_from_name(c,n)) return f; return nullptr; }

std::string TypeName(const Il2CppType* t){ if(!t) return{}; char* n=gApi.type_get_name(t); if(!n)return{}; std::string s(n); gApi.free_fn(n); return s; }

bool Invoke(const MethodInfo* m,void* instance,void** args,Il2CppObject*& out){
    out=nullptr; if(!m)return false; void* exc=nullptr; out=gApi.runtime_invoke(m,instance,args,&exc); return exc==nullptr;
}

bool CopyString(Il2CppObject* o,std::wstring& out){
    out.clear(); if(!o)return false; auto* c=gApi.object_get_class(o); const char* n=c?gApi.class_get_name(c):nullptr; if(!n||std::strcmp(n,"String"))return false;
    auto* s=reinterpret_cast<Il2CppString*>(o); int len=gApi.string_length(s); const wchar_t* ch=gApi.string_chars(s); if(len<0||len>32768||!ch)return false; out.assign(ch,ch+len); return true;
}

std::wstring ObjText(Il2CppObject* o){
    if(!o)return L"<null>"; std::wstring s; if(CopyString(o,s))return s; auto* c=gApi.object_get_class(o); const char* n=c?gApi.class_get_name(c):nullptr; void* p=gApi.object_unbox(o);
    if(n&&p){ if(!std::strcmp(n,"Int32"))return std::to_wstring(*reinterpret_cast<std::int32_t*>(p)); if(!std::strcmp(n,"UInt32"))return std::to_wstring(*reinterpret_cast<std::uint32_t*>(p)); if(!std::strcmp(n,"Int64"))return std::to_wstring(*reinterpret_cast<std::int64_t*>(p)); if(!std::strcmp(n,"Boolean"))return *reinterpret_cast<std::uint8_t*>(p)?L"1":L"0"; }
    return n?L"<"+W(n)+L">":L"<object>";
}

bool ObjMember(Il2CppObject* o,const char* member,Il2CppObject*& out){
    out=nullptr; if(!o)return false; auto* c=gApi.object_get_class(o); if(!c)return false;
    std::string getter="get_"+std::string(member);
    if(const MethodInfo* m=Method(c,getter.c_str(),0)) if(Invoke(m,o,nullptr,out)&&out)return true;
    if(auto* f=Field(c,member)){
        const std::string t=TypeName(gApi.field_get_type(f));
        if(t.find("System.Int")==std::string::npos && t.find("System.UInt")==std::string::npos && t!="System.Single" && t!="System.Double" && t!="System.Boolean"){
            gApi.field_get_value(o,f,&out); if(out)return true;
        }
    }
    Il2CppString* key=gApi.string_new(member);
    if(key){ for(const char* mn:{"get_Item","GetValue","Get","RawGet"}) if(const MethodInfo* m=Method(c,mn,1)){ void* args[]={&key}; if(Invoke(m,o,args,out)&&out)return true; } }
    return false;
}

bool TextMember(Il2CppObject* o,const char* m,std::wstring& out){ Il2CppObject* v=nullptr; if(!ObjMember(o,m,v))return false; out=ObjText(v); return true; }

bool NumberMember(Il2CppObject* o,const char* member,double& out,bool& integral){
    out=0;integral=false; if(!o)return false; auto* c=gApi.object_get_class(o); if(!c)return false;
    auto boxedNumber=[&](Il2CppObject* b)->bool{ if(!b)return false; auto* bc=gApi.object_get_class(b); const char* n=bc?gApi.class_get_name(bc):nullptr; void* p=gApi.object_unbox(b); if(!n||!p)return false;
        if(!std::strcmp(n,"Int32")){out=*reinterpret_cast<std::int32_t*>(p);integral=true;return true;} if(!std::strcmp(n,"UInt32")){out=*reinterpret_cast<std::uint32_t*>(p);integral=true;return true;} if(!std::strcmp(n,"Int64")){out=static_cast<double>(*reinterpret_cast<std::int64_t*>(p));integral=true;return true;} if(!std::strcmp(n,"Single")){out=*reinterpret_cast<float*>(p);return true;} if(!std::strcmp(n,"Double")){out=*reinterpret_cast<double*>(p);return true;} return false; };
    std::string getter="get_"+std::string(member); Il2CppObject* b=nullptr; if(const MethodInfo* m=Method(c,getter.c_str(),0)) if(Invoke(m,o,nullptr,b)&&boxedNumber(b))return true;
    if(auto* f=Field(c,member)){ const std::string t=TypeName(gApi.field_get_type(f)); if(t=="System.Int32"){std::int32_t v{};gApi.field_get_value(o,f,&v);out=v;integral=true;return true;} if(t=="System.UInt32"){std::uint32_t v{};gApi.field_get_value(o,f,&v);out=v;integral=true;return true;} if(t=="System.Int64"){std::int64_t v{};gApi.field_get_value(o,f,&v);out=static_cast<double>(v);integral=true;return true;} if(t=="System.Single"){float v{};gApi.field_get_value(o,f,&v);out=v;return true;} if(t=="System.Double"){double v{};gApi.field_get_value(o,f,&v);out=v;return true;} }
    b=nullptr; if(ObjMember(o,member,b)&&boxedNumber(b))return true; return false;
}

std::wstring Num(double v,bool i){ if(i)return std::to_wstring(static_cast<long long>(v)); std::wostringstream os;os<<std::fixed<<std::setprecision(2)<<v;return os.str(); }

template<typename T> bool ReadLocal(const void* base,std::size_t off,T& out){ SIZE_T done=0; return base && ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const char*>(base)+off,&out,sizeof(out),&done)&&done==sizeof(out); }

bool PtrArray(Il2CppObject* a,std::vector<Il2CppObject*>& out){ out.clear(); std::uintptr_t n=0; if(!a||!ReadLocal(a,0x18,n)||n>4096)return false; for(std::uintptr_t i=0;i<n;i++){Il2CppObject* v=nullptr;if(!ReadLocal(a,0x20+i*sizeof(void*),v))return false;if(v)out.push_back(v);}return true; }

bool Singleton(Il2CppClass* c,Il2CppObject*& out){ out=nullptr; if(!c)return false; if(const MethodInfo* m=Method(c,"get_Instance",0)) if(IsStatic(m)&&Invoke(m,nullptr,nullptr,out)&&out)return true; for(const char* n:{"Instance","instance","_instance","m_Instance"}) if(auto* f=Field(c,n)){gApi.field_static_get_value(f,&out);if(out)return true;} return false; }

bool InvokeNoArg(Il2CppClass* c,const char* n,Il2CppObject*& out){ if(!c)return false; const MethodInfo* m=Method(c,n,0); if(!m)return false; void* instance=nullptr;Il2CppObject* inst=nullptr;if(!IsStatic(m)){if(!Singleton(c,inst))return false;instance=inst;}return Invoke(m,instance,nullptr,out); }

std::wstring ClassName(Il2CppObject* o){auto* c=o?gApi.object_get_class(o):nullptr;const char* n=c?gApi.class_get_name(c):nullptr;const char* ns=c?gApi.class_get_namespace(c):nullptr;if(!n)return L"?";return ns&&*ns?W(ns)+L"."+W(n):W(n);}

bool Leader(std::wstring& line){
    auto* s=Class("FGStudio.LuaSystem","LuaSystemSharedData"); if(!s)return false; const MethodInfo* m=Method(s,"get_LeaderRoleData",0); Il2CppObject* r=nullptr;if(!m||!IsStatic(m)||!Invoke(m,nullptr,nullptr,r)||!r)return false;
    std::wstring name;double role=0,map=0,x=0,y=0;bool ir=false,im=false,ix=false,iy=false;TextMember(r,"Name",name);bool hr=NumberMember(r,"RoleID",role,ir);bool hm=NumberMember(r,"MapID",map,im);bool hx=NumberMember(r,"PosX",x,ix),hy=NumberMember(r,"PosY",y,iy);
    std::wostringstream os;os<<L"PLAYER";if(!name.empty())os<<L" name=\""<<name<<L"\"";if(hr)os<<L" RoleID="<<Num(role,ir);if(hm)os<<L" MapID="<<Num(map,im);if(hx&&hy)os<<L" Pos="<<Num(x,ix)<<L","<<Num(y,iy);line=os.str();return true;
}

bool Enumerate(Il2CppObject* d,std::vector<std::pair<std::wstring,Il2CppObject*>>& out){
    out.clear();if(!d)return false;auto* dc=gApi.object_get_class(d);const MethodInfo* ge=Method(dc,"GetEnumerator",0);Il2CppObject* e=nullptr;if(!ge||!Invoke(ge,d,nullptr,e)||!e)return false;auto* ec=gApi.object_get_class(e);const MethodInfo* mn=Method(ec,"MoveNext",0);const MethodInfo* gc=Method(ec,"get_Current",0);if(!mn||!gc)return false;
    for(std::size_t i=0;i<4096;i++){Il2CppObject* b=nullptr;if(!Invoke(mn,e,nullptr,b)||!b)return false;void* p=gApi.object_unbox(b);if(!p||!*reinterpret_cast<std::uint8_t*>(p))break;Il2CppObject* cur=nullptr;if(!Invoke(gc,e,nullptr,cur)||!cur)continue;Il2CppObject* k=nullptr,*v=nullptr;ObjMember(cur,"Key",k);ObjMember(cur,"Value",v);if(!v)v=cur;out.push_back({k?ObjText(k):std::to_wstring(i),v});}
    return true;
}

std::wstring Describe(Il2CppObject* o,const std::wstring& key,std::size_t idx){
    std::wostringstream os;os<<L"["<<idx<<L"] key="<<key<<L" class="<<ClassName(o);for(const char* m:{"Name","Type","ResName"}){std::wstring v;if(TextMember(o,m,v)&&!v.empty())os<<L" "<<W(m)<<L"=\""<<v<<L"\"";}for(const char* m:{"RoleID","ID","NpcID","NPCID","ResID","TemplateID"}){double v=0;bool i=false;if(NumberMember(o,m,v,i))os<<L" "<<W(m)<<L"="<<Num(v,i);}double x=0,y=0;bool ix=false,iy=false;bool hx=NumberMember(o,"PosX",x,ix)||NumberMember(o,"X",x,ix)||NumberMember(o,"x",x,ix);bool hy=NumberMember(o,"PosY",y,iy)||NumberMember(o,"Y",y,iy)||NumberMember(o,"y",y,iy);Il2CppObject* pos=nullptr;if((!hx||!hy)&&ObjMember(o,"Position",pos)&&pos){if(!hx)hx=NumberMember(pos,"X",x,ix)||NumberMember(pos,"x",x,ix);if(!hy)hy=NumberMember(pos,"Y",y,iy)||NumberMember(pos,"y",y,iy);}if(hx&&hy)os<<L" Pos="<<Num(x,ix)<<L","<<Num(y,iy);return os.str();
}

ResultCode DumpNearby(std::wstring& detail){
    auto* game=Class("FGStudio.LuaSystem.API","LuaSystemAPI_Game");auto* shared=Class("FGStudio.LuaSystem","LuaSystemSharedData");Il2CppObject* r=nullptr;bool ok=game&&InvokeNoArg(game,"GetNearbyObjects",r);if((!ok||!r)&&shared){r=nullptr;ok=InvokeNoArg(shared,"GetNearbyObjects",r);}if(!ok)return ResultCode::MethodNotFound;if(!r)return ResultCode::NullResult;std::vector<std::pair<std::wstring,Il2CppObject*>> items;if(!Enumerate(r,items)){detail=L"GetNearbyObjects return="+ClassName(r)+L" nhưng không enumerate được";return ResultCode::EnumerationFailed;}std::wostringstream os;os<<L"=== NPC / OBJECT LIVE QUANH NHÂN VẬT — READ ONLY ===\n";std::wstring pl;if(Leader(pl))os<<pl<<L"\n";os<<L"GetNearbyObjects class="<<ClassName(r)<<L" count="<<items.size()<<L"\nTìm Name bắt đầu 'Xa Truyền' hoặc Type=NPC; key/RoleID/ID là ứng viên ID live.\n\n";std::size_t n=0;for(auto& kv:items)if(kv.second)os<<Describe(kv.second,kv.first,++n)<<L"\n";detail=os.str();return ResultCode::Ok;
}

bool FindUI(const char* name,Il2CppObject*& ui){ui=nullptr;auto* gui=Class("FGStudio.LuaSystem.API","LuaSystemAPI_GUI");if(!gui)return false;Il2CppString* s=gApi.string_new(name);for(const char* mn:{"FindUI","MainFindUI"})if(const MethodInfo* m=Method(gui,mn,1)){void* args[]={&s};if(IsStatic(m)&&Invoke(m,nullptr,args,ui)&&ui)return true;}return false;}

bool Children(Il2CppObject* o,std::vector<Il2CppObject*>& out){Il2CppObject* a=nullptr;if(!ObjMember(o,"CoreChildren",a)&&!ObjMember(o,"Children",a))return false;return PtrArray(a,out);}

bool Clickable(Il2CppObject* o){auto* c=o?gApi.object_get_class(o):nullptr;return c&&(Method(c,"HandleClickEvent",0)||Method(c,"get_PointerClickHandler",0));}

std::wstring DescText(Il2CppObject* root){std::wstring out;std::vector<std::pair<Il2CppObject*,int>> q{{root,0}};std::vector<Il2CppObject*> seen;while(!q.empty()&&seen.size()<128){auto p=q.back();q.pop_back();if(!p.first||p.second>5||std::find(seen.begin(),seen.end(),p.first)!=seen.end())continue;seen.push_back(p.first);if(p.first!=root){std::wstring t;if(TextMember(p.first,"Text",t)&&!t.empty()){if(!out.empty())out+=L" / ";out+=t;}}std::vector<Il2CppObject*> c;if(Children(p.first,c))for(auto* x:c)q.push_back({x,p.second+1});}return out;}

ResultCode DumpDialog(std::wstring& detail){
    Il2CppObject* root=nullptr;if(!FindUI("GameDialog",root)||!root){detail=L"=== GAMEDIALOG ===\nGameDialog chưa mở. Hãy tự click NPC Xa Truyền, giữ cửa sổ đang mở rồi chạy lại.";return ResultCode::GameDialogNotOpen;}struct Node{Il2CppObject* o;std::wstring path;int depth;};std::vector<Node> q{{root,L"GameDialog",0}};std::vector<Il2CppObject*> seen;std::wostringstream os;os<<L"=== GAMEDIALOG BUTTONS / SELECTIONS — READ ONLY ===\nRoot class="<<ClassName(root)<<L"\nVới GameDialog chuẩn, Tag chính là selectionID.\n\n";std::size_t buttons=0,nodes=0;while(!q.empty()&&seen.size()<4096){Node n=std::move(q.back());q.pop_back();if(!n.o||n.depth>24||std::find(seen.begin(),seen.end(),n.o)!=seen.end())continue;seen.push_back(n.o);nodes++;std::wstring nm;TextMember(n.o,"Name",nm);std::wstring path=n.path;if(!nm.empty())path+=L"/"+nm;if(Clickable(n.o)){std::wstring text,tag,handler;TextMember(n.o,"Text",text);if(text.empty())text=DescText(n.o);Il2CppObject* to=nullptr;if(ObjMember(n.o,"Tag",to)&&to)tag=ObjText(to);TextMember(n.o,"PointerClickHandler",handler);if(!text.empty()||!tag.empty()||!handler.empty()){os<<L"["<<++buttons<<L"] text=\""<<text<<L"\"";if(!tag.empty())os<<L" Tag/selectionID="<<tag;if(!handler.empty())os<<L" handler=\""<<handler<<L"\"";os<<L"\n    path="<<path<<L"\n";}}std::vector<Il2CppObject*> kids;if(Children(n.o,kids))for(auto* x:kids)q.push_back({x,path,n.depth+1});}os<<L"\nSUMMARY nodes="<<nodes<<L" clickable_with_data="<<buttons<<L"\n";detail=os.str();return ResultCode::Ok;
}

bool EnsureShared(){if(gShared)return true;wchar_t n[96]{};MappingName(GetCurrentProcessId(),n,_countof(n));gMapping=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,n);if(!gMapping)return false;gShared=reinterpret_cast<SharedBlock*>(MapViewOfFile(gMapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(SharedBlock)));return gShared&&gShared->magic==kMagic&&gShared->protocolVersion==kProtocolVersion&&gShared->targetPid==GetCurrentProcessId();}

void Process(){if(!EnsureShared())return;LONG seq=gShared->requestSeq;if(seq<=0||seq==gShared->completedSeq)return;if(InterlockedCompareExchange(&gShared->bridgeBusy,1,0)!=0)return;ResultCode rc=ResultCode::InternalError;std::wstring detail;if(GetCurrentThreadId()!=gShared->targetTid){rc=ResultCode::WrongThread;detail=L"Sai callback thread";}else{std::wstring why;if(!gApi.Load(why)){rc=GetModuleHandleW(L"GameAssembly.dll")?ResultCode::Il2CppExportMissing:ResultCode::GameAssemblyMissing;detail=why;}else switch(gShared->command){case Command::ValidateContext:rc=ResultCode::Ok;detail=L"PASS READ-ONLY: đúng window thread; build này không chứa command gameplay mutation.";break;case Command::DumpNearbyObjects:rc=DumpNearby(detail);break;case Command::DumpGameDialog:rc=DumpDialog(detail);break;case Command::ReadPlayerState:{std::wstring p;if(Leader(p)){rc=ResultCode::Ok;detail=p;}else{rc=ResultCode::FieldReadFailed;detail=L"Không đọc được LeaderRoleData";}}break;default:detail=L"Command không hợp lệ";break;}}gShared->result=rc;wcsncpy_s(gShared->detail,_countof(gShared->detail),detail.c_str(),_TRUNCATE);MemoryBarrier();InterlockedExchange(&gShared->completedSeq,seq);InterlockedExchange(&gShared->bridgeBusy,0);}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlnpGetMessageHook(int code,WPARAM wParam,LPARAM lParam){(void)wParam;if(code>=0&&lParam){const MSG* m=reinterpret_cast<const MSG*>(lParam);if(m->message==kWakeMessage)Process();}return CallNextHookEx(nullptr,code,wParam,lParam);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH)DisableThreadLibraryCalls(h);if(reason==DLL_PROCESS_DETACH){if(gShared)UnmapViewOfFile(gShared);if(gMapping)CloseHandle(gMapping);gShared=nullptr;gMapping=nullptr;}return TRUE;}
