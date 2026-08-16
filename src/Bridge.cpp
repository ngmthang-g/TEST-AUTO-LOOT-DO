#include "Protocol.h"

#include <Windows.h>
#include <array>
#include <cstdint>
#include <cwchar>
#include <string>
#include <string_view>

using namespace remoteloopoc;

namespace {

struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppObject;
struct Il2CppType;
struct Il2CppThread;
struct MethodInfo;
struct FieldInfo;

constexpr std::uint32_t kMethodAttributeStatic = 0x0010;

struct Il2CppApi {
    HMODULE gameAssembly{};

    Il2CppDomain* (*domain_get)(){};
    const Il2CppAssembly** (*domain_get_assemblies)(Il2CppDomain*, std::size_t*){};
    const Il2CppImage* (*assembly_get_image)(const Il2CppAssembly*){};
    std::size_t (*image_get_class_count)(const Il2CppImage*){};
    Il2CppClass* (*image_get_class)(const Il2CppImage*, std::size_t){};
    const char* (*class_get_name)(Il2CppClass*){};
    const char* (*class_get_namespace)(Il2CppClass*){};
    const MethodInfo* (*class_get_methods)(Il2CppClass*, void**){};
    const MethodInfo* (*class_get_method_from_name)(Il2CppClass*, const char*, int){};
    FieldInfo* (*class_get_field_from_name)(Il2CppClass*, const char*){};
    const char* (*method_get_name)(const MethodInfo*){};
    std::uint32_t (*method_get_param_count)(const MethodInfo*){};
    const Il2CppType* (*method_get_param)(const MethodInfo*, std::uint32_t){};
    const Il2CppType* (*method_get_return_type)(const MethodInfo*){};
    std::uint32_t (*method_get_flags)(const MethodInfo*, std::uint32_t*){};
    char* (*type_get_name)(const Il2CppType*){};
    const Il2CppType* (*field_get_type)(FieldInfo*){};
    void (*field_get_value)(Il2CppObject*, FieldInfo*, void*){};
    void (*field_static_get_value)(FieldInfo*, void*){};
    Il2CppObject* (*runtime_invoke)(const MethodInfo*, void*, void**, Il2CppObject**){};
    void* (*object_unbox)(Il2CppObject*){};
    Il2CppClass* (*object_get_class)(Il2CppObject*){};
    Il2CppThread* (*thread_current)(){};
};

SharedBlock* gShared = nullptr;
HANDLE gMapping = nullptr;
LONG gLastHandledSequence = 0;

std::wstring Utf8ToWide(const char* text) {
    if (!text || !*text) return L"";
    const int needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (needed <= 1) return L"";
    std::wstring out(static_cast<std::size_t>(needed - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, out.data(), needed);
    return out;
}

void SetDetail(SharedBlock& s, const std::wstring& detail) {
    wcsncpy_s(s.detail, detail.c_str(), _TRUNCATE);
}

std::wstring MappingName(DWORD pid) {
    return std::wstring(kMappingPrefix) + std::to_wstring(pid);
}

bool EnsureShared() {
    if (gShared) return true;
    const auto name = MappingName(GetCurrentProcessId());
    gMapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (!gMapping) return false;
    gShared = static_cast<SharedBlock*>(MapViewOfFile(gMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    return gShared != nullptr;
}

template <typename T>
bool ResolveExport(HMODULE module, const char* name, T& out) {
    out = reinterpret_cast<T>(GetProcAddress(module, name));
    return out != nullptr;
}

bool ResolveIl2Cpp(Il2CppApi& a, std::wstring& why) {
    a.gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
    if (!a.gameAssembly) {
        why = L"GameAssembly.dll not loaded";
        return false;
    }

#define NEED_EXPORT(field, name) \
    if (!ResolveExport(a.gameAssembly, name, a.field)) { why = L"Missing IL2CPP export: " L##name; return false; }

    NEED_EXPORT(domain_get, "il2cpp_domain_get");
    NEED_EXPORT(domain_get_assemblies, "il2cpp_domain_get_assemblies");
    NEED_EXPORT(assembly_get_image, "il2cpp_assembly_get_image");
    NEED_EXPORT(image_get_class_count, "il2cpp_image_get_class_count");
    NEED_EXPORT(image_get_class, "il2cpp_image_get_class");
    NEED_EXPORT(class_get_name, "il2cpp_class_get_name");
    NEED_EXPORT(class_get_namespace, "il2cpp_class_get_namespace");
    NEED_EXPORT(class_get_methods, "il2cpp_class_get_methods");
    NEED_EXPORT(class_get_method_from_name, "il2cpp_class_get_method_from_name");
    NEED_EXPORT(class_get_field_from_name, "il2cpp_class_get_field_from_name");
    NEED_EXPORT(method_get_name, "il2cpp_method_get_name");
    NEED_EXPORT(method_get_param_count, "il2cpp_method_get_param_count");
    NEED_EXPORT(method_get_param, "il2cpp_method_get_param");
    NEED_EXPORT(method_get_return_type, "il2cpp_method_get_return_type");
    NEED_EXPORT(method_get_flags, "il2cpp_method_get_flags");
    NEED_EXPORT(type_get_name, "il2cpp_type_get_name");
    NEED_EXPORT(field_get_type, "il2cpp_field_get_type");
    NEED_EXPORT(field_get_value, "il2cpp_field_get_value");
    NEED_EXPORT(field_static_get_value, "il2cpp_field_static_get_value");
    NEED_EXPORT(runtime_invoke, "il2cpp_runtime_invoke");
    NEED_EXPORT(object_unbox, "il2cpp_object_unbox");
    NEED_EXPORT(object_get_class, "il2cpp_object_get_class");
    NEED_EXPORT(thread_current, "il2cpp_thread_current");
#undef NEED_EXPORT
    return true;
}

Il2CppClass* FindClass(Il2CppApi& a, std::string_view wantedName, std::string_view wantedNamespace = {}) {
    auto* domain = a.domain_get();
    if (!domain) return nullptr;
    std::size_t assemblyCount = 0;
    const auto** assemblies = a.domain_get_assemblies(domain, &assemblyCount);
    if (!assemblies) return nullptr;

    for (std::size_t ai = 0; ai < assemblyCount; ++ai) {
        const auto* image = a.assembly_get_image(assemblies[ai]);
        if (!image) continue;
        const auto count = a.image_get_class_count(image);
        for (std::size_t ci = 0; ci < count; ++ci) {
            auto* klass = a.image_get_class(image, ci);
            if (!klass) continue;
            const char* n = a.class_get_name(klass);
            const char* ns = a.class_get_namespace(klass);
            if (!n || wantedName != n) continue;
            if (!wantedNamespace.empty() && (!ns || wantedNamespace != ns)) continue;
            return klass;
        }
    }
    return nullptr;
}

const MethodInfo* FindMethodAnyCount(Il2CppApi& a, Il2CppClass* klass, const char* wantedName) {
    if (!klass) return nullptr;
    void* iter = nullptr;
    while (const auto* m = a.class_get_methods(klass, &iter)) {
        const char* n = a.method_get_name(m);
        if (n && std::string_view(n) == wantedName) return m;
    }
    return nullptr;
}

bool IsStatic(Il2CppApi& a, const MethodInfo* method) {
    std::uint32_t iflags = 0;
    return (a.method_get_flags(method, &iflags) & kMethodAttributeStatic) != 0;
}

std::string TypeName(Il2CppApi& a, const Il2CppType* type) {
    if (!type) return {};
    char* n = a.type_get_name(type);
    return n ? std::string(n) : std::string();
}

std::wstring MethodSignature(Il2CppApi& a, Il2CppClass* klass, const MethodInfo* m) {
    if (!m) return L"<null method>";
    std::wstring out;
    if (klass) {
        out += Utf8ToWide(a.class_get_namespace(klass));
        if (!out.empty()) out += L".";
        out += Utf8ToWide(a.class_get_name(klass));
        out += L".";
    }
    out += Utf8ToWide(a.method_get_name(m));
    out += L"(";
    const auto pc = a.method_get_param_count(m);
    for (std::uint32_t i = 0; i < pc; ++i) {
        if (i) out += L", ";
        const auto tn = TypeName(a, a.method_get_param(m, i));
        out += Utf8ToWide(tn.c_str());
    }
    out += L") -> ";
    const auto rn = TypeName(a, a.method_get_return_type(m));
    out += Utf8ToWide(rn.c_str());
    out += IsStatic(a, m) ? L" [static]" : L" [instance]";
    return out;
}

Il2CppObject* Invoke(Il2CppApi& a, const MethodInfo* method, void* instance, void** args, Il2CppObject** exception) {
    *exception = nullptr;
    return a.runtime_invoke(method, instance, args, exception);
}

Il2CppObject* ResolveInstance(Il2CppApi& a, Il2CppClass* klass, std::wstring& why) {
    if (!klass) return nullptr;

    if (const auto* getter = a.class_get_method_from_name(klass, "get_Instance", 0)) {
        if (IsStatic(a, getter)) {
            Il2CppObject* exc = nullptr;
            auto* obj = Invoke(a, getter, nullptr, nullptr, &exc);
            if (!exc && obj) return obj;
        }
    }

    constexpr const char* candidates[] = {"Instance", "instance", "_instance", "m_Instance"};
    for (const char* fieldName : candidates) {
        if (auto* field = a.class_get_field_from_name(klass, fieldName)) {
            Il2CppObject* obj = nullptr;
            a.field_static_get_value(field, &obj);
            if (obj) return obj;
        }
    }

    why = L"Instance method found but no usable singleton getter/static Instance field was found";
    return nullptr;
}

bool ValidateUnityContext(Il2CppApi& a, std::wstring& why) {
    if (!a.thread_current()) {
        why = L"il2cpp_thread_current() == null";
        return false;
    }

    auto* syncClass = FindClass(a, "SynchronizationContext", "System.Threading");
    if (!syncClass) {
        why = L"System.Threading.SynchronizationContext class not found";
        return false;
    }
    const auto* getter = a.class_get_method_from_name(syncClass, "get_Current", 0);
    if (!getter || !IsStatic(a, getter)) {
        why = L"SynchronizationContext.get_Current not found/static";
        return false;
    }
    Il2CppObject* exc = nullptr;
    auto* current = Invoke(a, getter, nullptr, nullptr, &exc);
    if (exc || !current) {
        why = L"SynchronizationContext.Current invoke failed/null";
        return false;
    }
    auto* currentClass = a.object_get_class(current);
    const char* className = currentClass ? a.class_get_name(currentClass) : nullptr;
    if (!className || std::string_view(className).find("UnitySynchronizationContext") == std::string_view::npos) {
        why = L"Current SynchronizationContext is not UnitySynchronizationContext";
        if (className) why += L" (got " + Utf8ToWide(className) + L")";
        return false;
    }
    return true;
}

struct ArgValue {
    std::int64_t i64{};
    std::uint64_t u64{};
    std::int32_t i32{};
    std::uint32_t u32{};
    bool b{};
};

bool BuildIntegerArgs(Il2CppApi& a, const MethodInfo* method, const std::array<std::int64_t, 3>& values,
                      std::array<ArgValue, 3>& storage, std::array<void*, 3>& args, std::wstring& why) {
    const auto count = a.method_get_param_count(method);
    if (count > values.size()) {
        why = L"Method has too many parameters for this PoC";
        return false;
    }
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto tn = TypeName(a, a.method_get_param(method, i));
        if (tn == "System.Int32" || tn == "int") {
            storage[i].i32 = static_cast<std::int32_t>(values[i]);
            args[i] = &storage[i].i32;
        } else if (tn == "System.UInt32") {
            storage[i].u32 = static_cast<std::uint32_t>(values[i]);
            args[i] = &storage[i].u32;
        } else if (tn == "System.Int64" || tn == "long") {
            storage[i].i64 = values[i];
            args[i] = &storage[i].i64;
        } else if (tn == "System.UInt64") {
            storage[i].u64 = static_cast<std::uint64_t>(values[i]);
            args[i] = &storage[i].u64;
        } else if (tn == "System.Boolean" || tn == "bool") {
            storage[i].b = values[i] != 0;
            args[i] = &storage[i].b;
        } else {
            why = L"Unsupported parameter type: " + Utf8ToWide(tn.c_str());
            return false;
        }
    }
    return true;
}

bool ReadIntegralObject(Il2CppApi& a, Il2CppObject* obj, const Il2CppType* declaredType, std::int64_t& out) {
    if (!obj || !declaredType) return false;
    const auto tn = TypeName(a, declaredType);
    void* p = a.object_unbox(obj);
    if (!p) return false;
    if (tn == "System.Int32" || tn == "int") { out = *static_cast<std::int32_t*>(p); return true; }
    if (tn == "System.UInt32") { out = *static_cast<std::uint32_t*>(p); return true; }
    if (tn == "System.Int64" || tn == "long") { out = *static_cast<std::int64_t*>(p); return true; }
    if (tn == "System.UInt64") { out = static_cast<std::int64_t>(*static_cast<std::uint64_t*>(p)); return true; }
    if (tn == "System.Boolean" || tn == "bool") { out = *static_cast<bool*>(p) ? 1 : 0; return true; }
    return false;
}

bool ReadIntegralMember(Il2CppApi& a, Il2CppObject* obj, const char* member, std::int64_t& out) {
    if (!obj) return false;
    auto* klass = a.object_get_class(obj);
    if (!klass) return false;

    if (auto* field = a.class_get_field_from_name(klass, member)) {
        const auto tn = TypeName(a, a.field_get_type(field));
        if (tn == "System.Int32" || tn == "int") {
            std::int32_t v{}; a.field_get_value(obj, field, &v); out = v; return true;
        }
        if (tn == "System.UInt32") {
            std::uint32_t v{}; a.field_get_value(obj, field, &v); out = v; return true;
        }
        if (tn == "System.Int64" || tn == "long") {
            std::int64_t v{}; a.field_get_value(obj, field, &v); out = v; return true;
        }
        if (tn == "System.UInt64") {
            std::uint64_t v{}; a.field_get_value(obj, field, &v); out = static_cast<std::int64_t>(v); return true;
        }
    }

    const std::string getterName = std::string("get_") + member;
    if (const auto* getter = a.class_get_method_from_name(klass, getterName.c_str(), 0)) {
        Il2CppObject* exc = nullptr;
        auto* boxed = Invoke(a, getter, obj, nullptr, &exc);
        if (!exc && boxed) return ReadIntegralObject(a, boxed, a.method_get_return_type(getter), out);
    }
    return false;
}

ResultCode ResolveLootMethods(Il2CppApi& a, std::wstring& detail) {
    auto* apiGame = FindClass(a, "LuaSystemAPI_Game", "FGStudio.LuaSystem");
    auto* shared = FindClass(a, "LuaSystemSharedData", "FGStudio.LuaSystem");
    if (!apiGame || !shared) {
        detail = L"Missing class: ";
        if (!apiGame) detail += L"FGStudio.LuaSystem.LuaSystemAPI_Game ";
        if (!shared) detail += L"FGStudio.LuaSystem.LuaSystemSharedData";
        return ResultCode::ClassNotFound;
    }

    const auto* click = FindMethodAnyCount(a, apiGame, "ClickToObject");
    const auto* pickup = FindMethodAnyCount(a, apiGame, "PickUpItemFromItemPack");
    const auto* buff = FindMethodAnyCount(a, apiGame, "HasBuff");
    const auto* nearest = FindMethodAnyCount(a, shared, "GetNearestItemPack");

    detail = L"Loot API signatures:\n";
    detail += L"  " + MethodSignature(a, apiGame, click) + L"\n";
    detail += L"  " + MethodSignature(a, apiGame, pickup) + L"\n";
    detail += L"  " + MethodSignature(a, apiGame, buff) + L"\n";
    detail += L"  " + MethodSignature(a, shared, nearest);

    return (click && pickup && buff && nearest) ? ResultCode::Ok : ResultCode::MethodNotFound;
}

ResultCode InvokeGameIntegerMethod(Il2CppApi& a, const char* methodName, int expectedParamCount,
                                   const std::array<std::int64_t, 3>& values, std::wstring& detail,
                                   Il2CppObject** resultObject = nullptr, const MethodInfo** resolvedMethod = nullptr) {
    auto* apiGame = FindClass(a, "LuaSystemAPI_Game", "FGStudio.LuaSystem");
    if (!apiGame) {
        detail = L"FGStudio.LuaSystem.LuaSystemAPI_Game not found";
        return ResultCode::ClassNotFound;
    }
    const auto* method = a.class_get_method_from_name(apiGame, methodName, expectedParamCount);
    if (!method) {
        const auto* any = FindMethodAnyCount(a, apiGame, methodName);
        detail = L"Method not found with expected parameter count. Found: " + MethodSignature(a, apiGame, any);
        return ResultCode::MethodNotFound;
    }

    void* instance = nullptr;
    if (!IsStatic(a, method)) {
        instance = ResolveInstance(a, apiGame, detail);
        if (!instance) return ResultCode::InstanceNotFound;
    }

    std::array<ArgValue, 3> storage{};
    std::array<void*, 3> args{};
    if (!BuildIntegerArgs(a, method, values, storage, args, detail)) {
        detail += L" | " + MethodSignature(a, apiGame, method);
        return ResultCode::SignatureUnsupported;
    }

    Il2CppObject* exc = nullptr;
    auto* ret = Invoke(a, method, instance, expectedParamCount ? args.data() : nullptr, &exc);
    if (exc) {
        detail = L"IL2CPP exception while invoking " + MethodSignature(a, apiGame, method);
        return ResultCode::InvokeException;
    }
    if (resultObject) *resultObject = ret;
    if (resolvedMethod) *resolvedMethod = method;
    detail = L"Invoked " + MethodSignature(a, apiGame, method);
    return ResultCode::Ok;
}

ResultCode ScanNearestPack(Il2CppApi& a, SharedBlock& s, std::wstring& detail) {
    auto* shared = FindClass(a, "LuaSystemSharedData", "FGStudio.LuaSystem");
    if (!shared) {
        detail = L"FGStudio.LuaSystem.LuaSystemSharedData not found";
        return ResultCode::ClassNotFound;
    }
    const auto* method = FindMethodAnyCount(a, shared, "GetNearestItemPack");
    if (!method) {
        detail = L"GetNearestItemPack not found";
        return ResultCode::MethodNotFound;
    }
    if (a.method_get_param_count(method) != 0) {
        detail = L"Scanner intentionally refuses to guess parameters. Runtime signature: " + MethodSignature(a, shared, method);
        return ResultCode::SignatureUnsupported;
    }

    void* instance = nullptr;
    if (!IsStatic(a, method)) {
        instance = ResolveInstance(a, shared, detail);
        if (!instance) return ResultCode::InstanceNotFound;
    }

    Il2CppObject* exc = nullptr;
    auto* pack = Invoke(a, method, instance, nullptr, &exc);
    if (exc) {
        detail = L"Exception invoking " + MethodSignature(a, shared, method);
        return ResultCode::InvokeException;
    }
    if (!pack) {
        detail = L"GetNearestItemPack returned null";
        return ResultCode::NoPack;
    }

    std::int64_t roleId = 0;
    if (!ReadIntegralMember(a, pack, "RoleID", roleId)) {
        detail = L"Pack object returned, but RoleID field/property could not be read";
        return ResultCode::FieldReadFailed;
    }
    std::int64_t type = 0;
    ReadIntegralMember(a, pack, "Type", type);

    s.out0 = roleId;
    s.out1 = type;
    detail = L"Nearest ItemPack RoleID=" + std::to_wstring(roleId) + L", Type=" + std::to_wstring(type) +
             L". PoC uses RoleID as candidate itemPackID because shipped loot tracks current pack by RoleID; runtime test remains authoritative.";
    return ResultCode::Ok;
}

void HandleCommand(SharedBlock& s) {
    s.result = ResultCode::InternalError;
    s.out0 = s.out1 = s.out2 = s.out3 = 0;
    s.detail[0] = L'\0';

    if (s.magic != kMagic || s.version != kProtocolVersion) {
        s.result = ResultCode::BadProtocol;
        SetDetail(s, L"Shared protocol mismatch");
        return;
    }

    Il2CppApi api{};
    std::wstring detail;
    if (!ResolveIl2Cpp(api, detail)) {
        s.result = GetModuleHandleW(L"GameAssembly.dll") ? ResultCode::Il2CppExportMissing : ResultCode::GameAssemblyMissing;
        SetDetail(s, detail);
        return;
    }

    if (!ValidateUnityContext(api, detail)) {
        s.result = api.thread_current() ? ResultCode::NotUnitySynchronizationContext : ResultCode::NotManagedThread;
        SetDetail(s, detail);
        return;
    }

    switch (s.command) {
        case Command::ValidateContext:
            s.result = ResultCode::Ok;
            SetDetail(s, L"PASS: hook is on an IL2CPP-managed UnitySynchronizationContext thread. No gameplay mutation executed.");
            return;

        case Command::ResolveLootApi:
            s.result = ResolveLootMethods(api, detail);
            SetDetail(s, detail);
            return;

        case Command::ScanNearestPack:
            s.result = ScanNearestPack(api, s, detail);
            SetDetail(s, detail);
            return;

        case Command::ClickObject: {
            s.result = InvokeGameIntegerMethod(api, "ClickToObject", 1, {s.arg0, 0, 0}, detail);
            SetDetail(s, detail + L". No MoveTo/MoveToEx was called by this PoC.");
            return;
        }

        case Command::DirectPickupAll: {
            s.result = InvokeGameIntegerMethod(api, "PickUpItemFromItemPack", 3, {s.arg0, -1, 1}, detail);
            SetDetail(s, detail + L" with (itemPackID, -1, 1). No movement call was issued by this PoC.");
            return;
        }

        case Command::HasCanKhonHoBuff: {
            Il2CppObject* ret = nullptr;
            const MethodInfo* method = nullptr;
            s.result = InvokeGameIntegerMethod(api, "HasBuff", 1, {30008009, 0, 0}, detail, &ret, &method);
            if (s.result == ResultCode::Ok) {
                std::int64_t value = 0;
                if (!ReadIntegralObject(api, ret, api.method_get_return_type(method), value)) {
                    s.result = ResultCode::FieldReadFailed;
                    detail += L"; return value could not be unboxed as bool/integer";
                } else {
                    s.out0 = value;
                    detail += value ? L"; buff 30008009 PRESENT" : L"; buff 30008009 ABSENT";
                }
            }
            SetDetail(s, detail);
            return;
        }

        default:
            s.result = ResultCode::InternalError;
            SetDetail(s, L"Unknown command");
            return;
    }
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK RemoteLootGetMessageHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code >= 0 && lParam && EnsureShared()) {
        auto* msg = reinterpret_cast<MSG*>(lParam);
        if (msg->message == kWakeMessage) {
            const LONG seq = gShared->requestSequence;
            if (seq != 0 && seq != gLastHandledSequence) {
                gLastHandledSequence = seq;
                HandleCommand(*gShared);
                MemoryBarrier();
                InterlockedExchange(&gShared->responseSequence, seq);
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH) {
        if (gShared) {
            UnmapViewOfFile(gShared);
            gShared = nullptr;
        }
        if (gMapping) {
            CloseHandle(gMapping);
            gMapping = nullptr;
        }
    }
    return TRUE;
}
