#include "protocol.h"

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <initializer_list>

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

template <typename T>
bool Resolve(HMODULE module, const char* name, T& out) {
    out = reinterpret_cast<T>(GetProcAddress(module, name));
    return out != nullptr;
}

template <typename T>
bool ReadLocal(const void* base, std::size_t offset, T& out) {
    if (!base) return false;
    SIZE_T done = 0;
    const auto* address = reinterpret_cast<const unsigned char*>(base) + offset;
    return ReadProcessMemory(GetCurrentProcess(), address, &out, sizeof(out), &done) != FALSE &&
           done == sizeof(out);
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
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    bool (__cdecl* class_is_valuetype)(Il2CppClass*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppString*) = nullptr;
    const wchar_t* (__cdecl* string_chars)(Il2CppString*) = nullptr;
    Il2CppString* (__cdecl* string_new)(const char*) = nullptr;

    bool Load(std::wstring& why) {
        if (module) return true;
        module = GetModuleHandleW(L"GameAssembly.dll");
        if (!module) {
            why = L"GameAssembly.dll chưa sẵn sàng";
            return false;
        }
#define NEED(field, exportName) do { if (!Resolve(module, exportName, field)) { why = L"Thiếu IL2CPP export: " L##exportName; return false; } } while (0)
        NEED(domain_get, "il2cpp_domain_get");
        NEED(domain_assembly_open, "il2cpp_domain_assembly_open");
        NEED(assembly_get_image, "il2cpp_assembly_get_image");
        NEED(class_from_name, "il2cpp_class_from_name");
        NEED(class_get_parent, "il2cpp_class_get_parent");
        NEED(class_get_method_from_name, "il2cpp_class_get_method_from_name");
        NEED(class_get_name, "il2cpp_class_get_name");
        NEED(class_get_namespace, "il2cpp_class_get_namespace");
        NEED(image_get_class_count, "il2cpp_image_get_class_count");
        NEED(image_get_class, "il2cpp_image_get_class");
        NEED(class_get_field_from_name, "il2cpp_class_get_field_from_name");
        NEED(field_get_type, "il2cpp_field_get_type");
        NEED(field_get_value, "il2cpp_field_get_value");
        NEED(field_static_get_value, "il2cpp_field_static_get_value");
        NEED(method_get_flags, "il2cpp_method_get_flags");
        NEED(type_get_name, "il2cpp_type_get_name");
        NEED(free_fn, "il2cpp_free");
        NEED(runtime_invoke, "il2cpp_runtime_invoke");
        NEED(object_unbox, "il2cpp_object_unbox");
        NEED(object_get_class, "il2cpp_object_get_class");
        NEED(class_is_valuetype, "il2cpp_class_is_valuetype");
        NEED(string_length, "il2cpp_string_length");
        NEED(string_chars, "il2cpp_string_chars");
        NEED(string_new, "il2cpp_string_new");
#undef NEED
        return true;
    }
} gApi;

const Il2CppImage* Image() {
    Il2CppDomain* domain = gApi.domain_get ? gApi.domain_get() : nullptr;
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = gApi.domain_assembly_open(domain, "Assembly-CSharp");
    if (!assembly) assembly = gApi.domain_assembly_open(domain, "Assembly-CSharp.dll");
    return assembly ? gApi.assembly_get_image(assembly) : nullptr;
}

bool IsStatic(const MethodInfo* method) {
    if (!method) return false;
    std::uint32_t iflags = 0;
    return (gApi.method_get_flags(method, &iflags) & 0x0010u) != 0;
}

const MethodInfo* Method(Il2CppClass* klass, const char* name, int argc) {
    for (Il2CppClass* current = klass; current; current = gApi.class_get_parent(current)) {
        if (const MethodInfo* method = gApi.class_get_method_from_name(current, name, argc)) return method;
    }
    return nullptr;
}

FieldInfo* Field(Il2CppClass* klass, const char* name) {
    for (Il2CppClass* current = klass; current; current = gApi.class_get_parent(current)) {
        if (FieldInfo* field = gApi.class_get_field_from_name(current, name)) return field;
    }
    return nullptr;
}

Il2CppClass* Class(const char* nameSpace, const char* name) {
    const Il2CppImage* image = Image();
    if (!image) return nullptr;
    if (nameSpace) {
        if (Il2CppClass* klass = gApi.class_from_name(image, nameSpace, name)) return klass;
    }
    const std::size_t count = gApi.image_get_class_count(image);
    if (count == 0 || count > 65536) return nullptr;
    for (std::size_t i = 0; i < count; ++i) {
        Il2CppClass* klass = gApi.image_get_class(image, i);
        const char* className = klass ? gApi.class_get_name(klass) : nullptr;
        if (className && std::strcmp(className, name) == 0) return klass;
    }
    return nullptr;
}

std::string TypeName(const Il2CppType* type) {
    if (!type) return {};
    char* raw = gApi.type_get_name(type);
    if (!raw) return {};
    std::string result(raw);
    gApi.free_fn(raw);
    return result;
}

bool Invoke(const MethodInfo* method, void* instance, void** args, Il2CppObject*& out) {
    out = nullptr;
    if (!method) return false;
    void* exception = nullptr;
    out = gApi.runtime_invoke(method, instance, args, &exception);
    return exception == nullptr;
}

void* ManagedThis(Il2CppObject* object) {
    if (!object) return nullptr;
    Il2CppClass* klass = gApi.object_get_class(object);
    if (klass && gApi.class_is_valuetype(klass)) {
        void* unboxed = gApi.object_unbox(object);
        if (unboxed) return unboxed;
    }
    return object;
}

bool CopyString(Il2CppObject* object, std::wstring& out) {
    out.clear();
    if (!object) return false;
    Il2CppClass* klass = gApi.object_get_class(object);
    const char* className = klass ? gApi.class_get_name(klass) : nullptr;
    if (!className || std::strcmp(className, "String") != 0) return false;
    auto* stringObject = reinterpret_cast<Il2CppString*>(object);
    const int length = gApi.string_length(stringObject);
    const wchar_t* chars = gApi.string_chars(stringObject);
    if (length < 0 || length > 32768 || !chars) return false;
    out.assign(chars, chars + length);
    return true;
}

std::wstring FloatingText(double value) {
    if (!std::isfinite(value)) {
        std::wostringstream out;
        out << value;
        return out.str();
    }
    const double rounded = std::round(value);
    if (std::fabs(value - rounded) < 1e-9 && std::fabs(rounded) <= 9007199254740991.0) {
        return std::to_wstring(static_cast<long long>(rounded));
    }
    std::wostringstream out;
    out << std::setprecision(15) << value;
    return out.str();
}

std::wstring ObjText(Il2CppObject* object) {
    if (!object) return L"<null>";
    std::wstring text;
    if (CopyString(object, text)) return text;

    Il2CppClass* klass = gApi.object_get_class(object);
    const char* className = klass ? gApi.class_get_name(klass) : nullptr;
    void* raw = gApi.object_unbox(object);
    if (className && raw) {
        if (std::strcmp(className, "Int32") == 0) return std::to_wstring(*reinterpret_cast<std::int32_t*>(raw));
        if (std::strcmp(className, "UInt32") == 0) return std::to_wstring(*reinterpret_cast<std::uint32_t*>(raw));
        if (std::strcmp(className, "Int64") == 0) return std::to_wstring(*reinterpret_cast<std::int64_t*>(raw));
        if (std::strcmp(className, "UInt64") == 0) return std::to_wstring(*reinterpret_cast<std::uint64_t*>(raw));
        if (std::strcmp(className, "Int16") == 0) return std::to_wstring(*reinterpret_cast<std::int16_t*>(raw));
        if (std::strcmp(className, "UInt16") == 0) return std::to_wstring(*reinterpret_cast<std::uint16_t*>(raw));
        if (std::strcmp(className, "SByte") == 0) return std::to_wstring(*reinterpret_cast<std::int8_t*>(raw));
        if (std::strcmp(className, "Byte") == 0) return std::to_wstring(*reinterpret_cast<std::uint8_t*>(raw));
        if (std::strcmp(className, "Boolean") == 0) return *reinterpret_cast<std::uint8_t*>(raw) ? L"1" : L"0";
        if (std::strcmp(className, "Single") == 0) return FloatingText(*reinterpret_cast<float*>(raw));
        if (std::strcmp(className, "Double") == 0) return FloatingText(*reinterpret_cast<double*>(raw));
    }
    return className ? L"<" + W(className) + L">" : L"<object>";
}

bool ObjMember(Il2CppObject* object, const char* member, Il2CppObject*& out) {
    out = nullptr;
    if (!object) return false;
    Il2CppClass* klass = gApi.object_get_class(object);
    if (!klass) return false;

    const std::string getter = std::string("get_") + member;
    if (const MethodInfo* method = Method(klass, getter.c_str(), 0)) {
        if (Invoke(method, ManagedThis(object), nullptr, out) && out) return true;
    }

    if (!gApi.class_is_valuetype(klass)) {
        if (FieldInfo* field = Field(klass, member)) {
            const std::string type = TypeName(gApi.field_get_type(field));
            if (type.find("System.Int") == std::string::npos &&
                type.find("System.UInt") == std::string::npos &&
                type != "System.Single" && type != "System.Double" && type != "System.Boolean") {
                gApi.field_get_value(object, field, &out);
                if (out) return true;
            }
        }
    }

    Il2CppString* key = gApi.string_new(member);
    if (key) {
        for (const char* methodName : {"get_Item", "GetValue", "Get", "RawGet"}) {
            if (const MethodInfo* method = Method(klass, methodName, 1)) {
                void* args[] = {&key};
                if (Invoke(method, ManagedThis(object), args, out) && out) return true;
            }
        }
    }
    return false;
}

bool TextMember(Il2CppObject* object, const char* member, std::wstring& out) {
    out.clear();
    Il2CppObject* value = nullptr;
    if (!ObjMember(object, member, value) || !value) return false;
    out = ObjText(value);
    return true;
}

bool BoxedNumber(Il2CppObject* boxed, double& out, bool& integral) {
    out = 0.0;
    integral = false;
    if (!boxed) return false;
    Il2CppClass* klass = gApi.object_get_class(boxed);
    const char* name = klass ? gApi.class_get_name(klass) : nullptr;
    void* raw = gApi.object_unbox(boxed);
    if (!name || !raw) return false;

    if (std::strcmp(name, "Int32") == 0) { out = *reinterpret_cast<std::int32_t*>(raw); integral = true; return true; }
    if (std::strcmp(name, "UInt32") == 0) { out = *reinterpret_cast<std::uint32_t*>(raw); integral = true; return true; }
    if (std::strcmp(name, "Int64") == 0) { out = static_cast<double>(*reinterpret_cast<std::int64_t*>(raw)); integral = true; return true; }
    if (std::strcmp(name, "UInt64") == 0) { out = static_cast<double>(*reinterpret_cast<std::uint64_t*>(raw)); integral = true; return true; }
    if (std::strcmp(name, "Int16") == 0) { out = *reinterpret_cast<std::int16_t*>(raw); integral = true; return true; }
    if (std::strcmp(name, "UInt16") == 0) { out = *reinterpret_cast<std::uint16_t*>(raw); integral = true; return true; }
    if (std::strcmp(name, "Byte") == 0) { out = *reinterpret_cast<std::uint8_t*>(raw); integral = true; return true; }
    if (std::strcmp(name, "Boolean") == 0) { out = *reinterpret_cast<std::uint8_t*>(raw) ? 1.0 : 0.0; integral = true; return true; }
    if (std::strcmp(name, "Single") == 0) { out = *reinterpret_cast<float*>(raw); return true; }
    if (std::strcmp(name, "Double") == 0) { out = *reinterpret_cast<double*>(raw); return true; }
    return false;
}

bool NumberMember(Il2CppObject* object, const char* member, double& out, bool& integral) {
    out = 0.0;
    integral = false;
    if (!object) return false;
    Il2CppClass* klass = gApi.object_get_class(object);
    if (!klass) return false;

    const std::string getter = std::string("get_") + member;
    if (const MethodInfo* method = Method(klass, getter.c_str(), 0)) {
        Il2CppObject* boxed = nullptr;
        if (Invoke(method, ManagedThis(object), nullptr, boxed) && BoxedNumber(boxed, out, integral)) return true;
    }

    if (!gApi.class_is_valuetype(klass)) {
        if (FieldInfo* field = Field(klass, member)) {
            const std::string type = TypeName(gApi.field_get_type(field));
            if (type == "System.Int32") { std::int32_t v{}; gApi.field_get_value(object, field, &v); out = v; integral = true; return true; }
            if (type == "System.UInt32") { std::uint32_t v{}; gApi.field_get_value(object, field, &v); out = v; integral = true; return true; }
            if (type == "System.Int64") { std::int64_t v{}; gApi.field_get_value(object, field, &v); out = static_cast<double>(v); integral = true; return true; }
            if (type == "System.UInt64") { std::uint64_t v{}; gApi.field_get_value(object, field, &v); out = static_cast<double>(v); integral = true; return true; }
            if (type == "System.Single") { float v{}; gApi.field_get_value(object, field, &v); out = v; return true; }
            if (type == "System.Double") { double v{}; gApi.field_get_value(object, field, &v); out = v; return true; }
            if (type == "System.Boolean") { std::uint8_t v{}; gApi.field_get_value(object, field, &v); out = v ? 1.0 : 0.0; integral = true; return true; }
        }
    }

    Il2CppObject* boxed = nullptr;
    return ObjMember(object, member, boxed) && BoxedNumber(boxed, out, integral);
}

std::wstring Num(double value, bool integral) {
    if (integral) return std::to_wstring(static_cast<long long>(value));
    std::wostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

bool PtrArray(Il2CppObject* array, std::vector<Il2CppObject*>& out, std::size_t hardLimit = 4096) {
    out.clear();
    std::uintptr_t length = 0;
    if (!array || !ReadLocal(array, 0x18, length) || length > hardLimit) return false;
    out.reserve(static_cast<std::size_t>(length));
    for (std::uintptr_t i = 0; i < length; ++i) {
        Il2CppObject* value = nullptr;
        if (!ReadLocal(array, 0x20 + static_cast<std::size_t>(i) * sizeof(void*), value)) return false;
        if (value) out.push_back(value);
    }
    return true;
}

bool Singleton(Il2CppClass* klass, Il2CppObject*& out) {
    out = nullptr;
    if (!klass) return false;
    if (const MethodInfo* method = Method(klass, "get_Instance", 0)) {
        if (IsStatic(method) && Invoke(method, nullptr, nullptr, out) && out) return true;
    }
    for (const char* name : {"Instance", "instance", "_instance", "m_Instance"}) {
        if (FieldInfo* field = Field(klass, name)) {
            gApi.field_static_get_value(field, &out);
            if (out) return true;
        }
    }
    return false;
}

bool InvokeNoArg(Il2CppClass* klass, const char* name, Il2CppObject*& out) {
    out = nullptr;
    if (!klass) return false;
    const MethodInfo* method = Method(klass, name, 0);
    if (!method) return false;
    void* instance = nullptr;
    Il2CppObject* singleton = nullptr;
    if (!IsStatic(method)) {
        if (!Singleton(klass, singleton)) return false;
        instance = ManagedThis(singleton);
    }
    return Invoke(method, instance, nullptr, out);
}

std::wstring ClassName(Il2CppObject* object) {
    Il2CppClass* klass = object ? gApi.object_get_class(object) : nullptr;
    const char* name = klass ? gApi.class_get_name(klass) : nullptr;
    const char* nameSpace = klass ? gApi.class_get_namespace(klass) : nullptr;
    if (!name) return L"?";
    return nameSpace && *nameSpace ? W(nameSpace) + L"." + W(name) : W(name);
}

bool Leader(std::wstring& line) {
    line.clear();
    Il2CppClass* shared = Class("FGStudio.LuaSystem", "LuaSystemSharedData");
    if (!shared) return false;
    const MethodInfo* getter = Method(shared, "get_LeaderRoleData", 0);
    Il2CppObject* role = nullptr;
    if (!getter || !IsStatic(getter) || !Invoke(getter, nullptr, nullptr, role) || !role) return false;

    std::wstring name;
    double roleId = 0, mapId = 0, x = 0, y = 0;
    bool iRole = false, iMap = false, iX = false, iY = false;
    TextMember(role, "Name", name);
    const bool hasRole = NumberMember(role, "RoleID", roleId, iRole);
    const bool hasMap = NumberMember(role, "MapID", mapId, iMap);
    const bool hasX = NumberMember(role, "PosX", x, iX);
    const bool hasY = NumberMember(role, "PosY", y, iY);

    std::wostringstream out;
    out << L"PLAYER";
    if (!name.empty()) out << L" name=\"" << name << L"\"";
    if (hasRole) out << L" RoleID=" << Num(roleId, iRole);
    if (hasMap) out << L" MapID=" << Num(mapId, iMap);
    if (hasX && hasY) out << L" Pos=" << Num(x, iX) << L"," << Num(y, iY);
    line = out.str();
    return true;
}

bool EnumerateDictionary(Il2CppObject* dictionary,
                         std::vector<std::pair<std::wstring, Il2CppObject*>>& out,
                         std::int64_t& expectedCount,
                         std::wstring& why) {
    out.clear();
    expectedCount = -1;
    why.clear();
    if (!dictionary) {
        why = L"dictionary=null";
        return false;
    }

    double countValue = 0.0;
    bool countIntegral = false;
    if (NumberMember(dictionary, "Count", countValue, countIntegral) && countIntegral && countValue >= 0.0 && countValue <= 4096.0) {
        expectedCount = static_cast<std::int64_t>(countValue);
    }

    Il2CppClass* dictionaryClass = gApi.object_get_class(dictionary);
    const MethodInfo* getEnumerator = Method(dictionaryClass, "GetEnumerator", 0);
    Il2CppObject* enumerator = nullptr;
    if (!getEnumerator || !Invoke(getEnumerator, ManagedThis(dictionary), nullptr, enumerator) || !enumerator) {
        why = L"GetEnumerator() thất bại";
        return false;
    }

    Il2CppClass* enumeratorClass = gApi.object_get_class(enumerator);
    const MethodInfo* moveNext = Method(enumeratorClass, "MoveNext", 0);
    const MethodInfo* getCurrent = Method(enumeratorClass, "get_Current", 0);
    if (!moveNext || !getCurrent) {
        why = L"Enumerator thiếu MoveNext/get_Current";
        return false;
    }

    const std::size_t hardLimit = expectedCount >= 0
        ? static_cast<std::size_t>(std::min<std::int64_t>(expectedCount + 4, 4096))
        : 4096;

    std::vector<std::wstring> seenKeys;
    bool reachedEnd = false;
    for (std::size_t i = 0; i < hardLimit; ++i) {
        Il2CppObject* boxedBool = nullptr;
        if (!Invoke(moveNext, ManagedThis(enumerator), nullptr, boxedBool) || !boxedBool) {
            why = L"MoveNext() invoke thất bại";
            return false;
        }
        double moveValue = 0.0;
        bool moveIntegral = false;
        if (!BoxedNumber(boxedBool, moveValue, moveIntegral)) {
            why = L"MoveNext() không trả Boolean";
            return false;
        }
        if (moveValue == 0.0) {
            reachedEnd = true;
            break;
        }

        Il2CppObject* current = nullptr;
        if (!Invoke(getCurrent, ManagedThis(enumerator), nullptr, current) || !current) {
            why = L"get_Current() thất bại";
            return false;
        }

        Il2CppObject* keyObject = nullptr;
        Il2CppObject* valueObject = nullptr;
        (void)ObjMember(current, "Key", keyObject);
        (void)ObjMember(current, "Value", valueObject);
        const std::wstring key = keyObject ? ObjText(keyObject) : (L"#" + std::to_wstring(i));

        if (keyObject && std::find(seenKeys.begin(), seenKeys.end(), key) != seenKeys.end()) {
            why = L"Enumerator lặp key '" + key + L"' — state không tiến";
            return false;
        }
        if (keyObject) seenKeys.push_back(key);
        if (!valueObject) valueObject = current;
        out.push_back({key, valueObject});

        if (expectedCount >= 0 && static_cast<std::int64_t>(out.size()) == expectedCount) {
            if (i + 1 >= hardLimit) break;
        }
    }

    if (expectedCount >= 0 && static_cast<std::int64_t>(out.size()) != expectedCount) {
        why = L"Dictionary.Count=" + std::to_wstring(expectedCount) +
              L" nhưng enumerate được " + std::to_wstring(out.size());
        return false;
    }
    if (expectedCount < 0 && !reachedEnd && out.size() >= hardLimit) {
        why = L"Enumerator chạm hard-limit 4096 mà chưa kết thúc";
        return false;
    }
    return true;
}

std::wstring Describe(Il2CppObject* object, const std::wstring& key, std::size_t index) {
    std::wostringstream out;
    out << L"[" << index << L"] key=" << key << L" class=" << ClassName(object);

    for (const char* member : {"Name", "Type", "ResName"}) {
        std::wstring value;
        if (TextMember(object, member, value) && !value.empty()) {
            out << L" " << W(member) << L"=\"" << value << L"\"";
        }
    }
    for (const char* member : {"RoleID", "ID", "NpcID", "NPCID", "ResID", "TemplateID"}) {
        double value = 0.0;
        bool integral = false;
        if (NumberMember(object, member, value, integral)) {
            out << L" " << W(member) << L"=" << Num(value, integral);
        }
    }

    double x = 0.0, y = 0.0;
    bool ix = false, iy = false;
    bool hasX = NumberMember(object, "PosX", x, ix) || NumberMember(object, "X", x, ix) || NumberMember(object, "x", x, ix);
    bool hasY = NumberMember(object, "PosY", y, iy) || NumberMember(object, "Y", y, iy) || NumberMember(object, "y", y, iy);
    Il2CppObject* position = nullptr;
    if ((!hasX || !hasY) && ObjMember(object, "Position", position) && position) {
        if (!hasX) hasX = NumberMember(position, "X", x, ix) || NumberMember(position, "x", x, ix);
        if (!hasY) hasY = NumberMember(position, "Y", y, iy) || NumberMember(position, "y", y, iy);
    }
    if (hasX && hasY) out << L" Pos=" << Num(x, ix) << L"," << Num(y, iy);
    return out.str();
}

ResultCode DumpNearby(std::wstring& detail) {
    Il2CppClass* game = Class("FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    Il2CppClass* shared = Class("FGStudio.LuaSystem", "LuaSystemSharedData");
    Il2CppObject* result = nullptr;
    bool ok = game && InvokeNoArg(game, "GetNearbyObjects", result);
    if ((!ok || !result) && shared) {
        result = nullptr;
        ok = InvokeNoArg(shared, "GetNearbyObjects", result);
    }
    if (!ok) {
        detail = L"Không gọi được GetNearbyObjects()";
        return ResultCode::MethodNotFound;
    }
    if (!result) {
        detail = L"GetNearbyObjects() trả null";
        return ResultCode::NullResult;
    }

    std::vector<std::pair<std::wstring, Il2CppObject*>> items;
    std::int64_t expectedCount = -1;
    std::wstring why;
    if (!EnumerateDictionary(result, items, expectedCount, why)) {
        detail = L"GetNearbyObjects return=" + ClassName(result) + L" nhưng enumerate thất bại: " + why;
        return ResultCode::EnumerationFailed;
    }

    std::wostringstream out;
    out << L"=== NPC / OBJECT LIVE QUANH NHÂN VẬT — READ ONLY ===\n";
    std::wstring player;
    if (Leader(player)) out << player << L"\n";
    out << L"GetNearbyObjects class=" << ClassName(result)
        << L" Dictionary.Count=" << expectedCount
        << L" enumerated=" << items.size() << L"\n";
    out << L"Tìm Name bắt đầu 'Xa Truyền' hoặc Type=NPC; key/RoleID/ID là ứng viên ID live.\n\n";

    std::size_t index = 0;
    for (const auto& item : items) {
        if (item.second) out << Describe(item.second, item.first, ++index) << L"\n";
    }
    detail = out.str();
    return ResultCode::Ok;
}

bool Children(Il2CppObject* object, std::vector<Il2CppObject*>& out) {
    Il2CppObject* array = nullptr;
    if (!ObjMember(object, "CoreChildren", array) && !ObjMember(object, "Children", array)) return false;
    return PtrArray(array, out, 512);
}

bool Clickable(Il2CppObject* object) {
    Il2CppClass* klass = object ? gApi.object_get_class(object) : nullptr;
    return klass && (Method(klass, "HandleClickEvent", 0) || Method(klass, "get_PointerClickHandler", 0));
}

std::wstring DescText(Il2CppObject* root) {
    std::wstring out;
    std::vector<std::pair<Il2CppObject*, int>> pending{{root, 0}};
    std::vector<Il2CppObject*> seen;
    while (!pending.empty() && seen.size() < 128) {
        auto [object, depth] = pending.back();
        pending.pop_back();
        if (!object || depth > 5 || std::find(seen.begin(), seen.end(), object) != seen.end()) continue;
        seen.push_back(object);
        if (object != root) {
            std::wstring text;
            if (TextMember(object, "Text", text) && !text.empty()) {
                if (!out.empty()) out += L" / ";
                out += text;
            }
        }
        std::vector<Il2CppObject*> children;
        if (Children(object, children)) {
            for (Il2CppObject* child : children) pending.push_back({child, depth + 1});
        }
    }
    return out;
}

void AppendPathPart(std::wstring& path, const std::wstring& part) {
    if (part.empty() || part == L"<null>") return;
    if (!path.empty()) path += L"/";
    path += part;
}

std::wstring ParentPath(Il2CppObject* object) {
    std::wstring path;

    Il2CppObject* parentArray = nullptr;
    if (ObjMember(object, "CoreParents", parentArray) && parentArray) {
        std::vector<Il2CppObject*> parents;
        if (PtrArray(parentArray, parents, 64)) {
            for (Il2CppObject* parent : parents) {
                std::wstring name;
                if (TextMember(parent, "Name", name)) AppendPathPart(path, name);
            }
            if (!path.empty()) return path;
        }
    }

    Il2CppObject* current = object;
    std::vector<Il2CppObject*> seen;
    for (int depth = 0; depth < 12 && current; ++depth) {
        if (std::find(seen.begin(), seen.end(), current) != seen.end()) break;
        seen.push_back(current);
        Il2CppObject* parent = nullptr;
        if (!ObjMember(current, "Parent", parent) || !parent) break;
        std::wstring name;
        if (TextMember(parent, "Name", name)) AppendPathPart(path, name);
        current = parent;
    }
    return path;
}

bool ContainsAny(const std::wstring& text, std::initializer_list<const wchar_t*> needles) {
    for (const wchar_t* needle : needles) {
        if (needle && text.find(needle) != std::wstring::npos) return true;
    }
    return false;
}

bool IsTravelLabel(const std::wstring& text) {
    if (text.empty()) return false;
    return ContainsAny(text, {
        L"Đại Lý", L"Lạc Dương", L"Tô Châu", L"Nam Hải",
        L"Thảo Nguyên", L"Hoàng Long Phủ", L"Miêu Cương", L"Thạch Lâm",
        L"Thiếu Lâm", L"Cái Bang", L"Nga My", L"Thiên Long", L"Thiên Sơn"
    });
}

struct UiRow {
    Il2CppObject* object = nullptr;
    std::wstring className;
    std::wstring name;
    std::wstring text;
    std::wstring tag;
    std::wstring handler;
    std::wstring path;
    bool clickable = false;
    bool travel = false;
};

bool EnumerateUiObjects(std::vector<Il2CppObject*>& objects,
                        std::int32_t& dictionaryCount,
                        std::uintptr_t& capacity,
                        std::wstring& why) {
    objects.clear();
    dictionaryCount = 0;
    capacity = 0;
    why.clear();

    Il2CppClass* uiObject = Class("FGStudio.LuaSystem.Base", "UIObject");
    if (!uiObject) {
        why = L"Không tìm thấy UIObject";
        return false;
    }
    FieldInfo* instances = Field(uiObject, "instances");
    if (!instances) {
        why = L"Không tìm thấy UIObject.instances";
        return false;
    }

    Il2CppObject* dictionary = nullptr;
    gApi.field_static_get_value(instances, &dictionary);
    Il2CppObject* entries = nullptr;
    if (!dictionary || !ReadLocal(dictionary, 0x18, entries) || !entries ||
        !ReadLocal(dictionary, 0x20, dictionaryCount) || dictionaryCount < 0 || dictionaryCount > 32768 ||
        !ReadLocal(entries, 0x18, capacity) || capacity > 32768) {
        why = L"UIObject.instances dictionary layout không hợp lệ";
        return false;
    }

    for (std::uintptr_t i = 0; i < capacity; ++i) {
        Il2CppObject* object = nullptr;
        const std::size_t entry = 0x20 + static_cast<std::size_t>(i) * 0x18;
        if (!ReadLocal(entries, entry + 0x10, object) || !object) continue;
        Il2CppClass* klass = nullptr;
        if (!ReadLocal(object, 0, klass) || !klass) continue;
        objects.push_back(object);
    }
    return true;
}

UiRow ReadUiRow(Il2CppObject* object) {
    UiRow row;
    row.object = object;
    row.className = ClassName(object);
    row.clickable = Clickable(object);
    (void)TextMember(object, "Name", row.name);
    (void)TextMember(object, "Text", row.text);
    if (row.clickable && row.text.empty()) row.text = DescText(object);

    Il2CppObject* tagObject = nullptr;
    if (ObjMember(object, "Tag", tagObject) && tagObject) row.tag = ObjText(tagObject);
    (void)TextMember(object, "PointerClickHandler", row.handler);
    row.path = ParentPath(object);
    const bool travelText = IsTravelLabel(row.text) || IsTravelLabel(row.name);
    const bool obviousNonTravelContext = ContainsAny(
        row.path,
        {L"TeamMemberList", L"MiniTeamFrame", L"RoleHeader", L"TeamRole", L"SpiritHeader"});
    row.travel = travelText && !obviousNonTravelContext;
    return row;
}

void EmitUiRow(std::wostringstream& out, const UiRow& row, std::size_t index, const wchar_t* prefix) {
    out << L"[" << prefix << L" " << index << L"] class=" << row.className;
    if (!row.name.empty()) out << L" name=\"" << row.name << L"\"";
    if (!row.text.empty()) out << L" text=\"" << row.text << L"\"";
    if (!row.tag.empty()) out << L" Tag/selectionID=" << row.tag;
    if (!row.handler.empty()) out << L" handler=\"" << row.handler << L"\"";
    out << L" clickable=" << (row.clickable ? 1 : 0) << L"\n";
    if (!row.path.empty()) out << L"    parents=" << row.path << L"\n";
}

ResultCode DumpUiLive(std::wstring& detail) {
    std::vector<Il2CppObject*> objects;
    std::int32_t dictionaryCount = 0;
    std::uintptr_t capacity = 0;
    std::wstring why;
    if (!EnumerateUiObjects(objects, dictionaryCount, capacity, why)) {
        detail = L"=== UI LIVE SCAN ===\n" + why;
        return ResultCode::EnumerationFailed;
    }

    std::vector<UiRow> travelRows;
    std::vector<UiRow> otherRows;
    std::size_t activeCount = 0;
    std::size_t clickableCount = 0;
    std::size_t textCount = 0;

    for (Il2CppObject* object : objects) {
        double active = 1.0;
        bool integral = true;
        if (NumberMember(object, "ActiveInHierarchy", active, integral) && active == 0.0) continue;
        ++activeCount;

        UiRow row = ReadUiRow(object);
        if (row.clickable) ++clickableCount;
        if (!row.text.empty()) ++textCount;

        if (row.travel) {
            travelRows.push_back(std::move(row));
            continue;
        }

        const bool usefulName = ContainsAny(row.name, {L"Button", L"Dialog", L"GameDialog", L"Selection", L"Select", L"List"});
        if (row.clickable || !row.text.empty() || !row.tag.empty() || usefulName) {
            if (otherRows.size() < 220) otherRows.push_back(std::move(row));
        }
    }

    std::wostringstream out;
    out << L"=== UI LIVE SCAN — READ ONLY ===\n";
    out << L"UIObject.instances Count=" << dictionaryCount
        << L" capacity=" << capacity
        << L" valuePointers=" << objects.size() << L"\n";
    out << L"active=" << activeCount
        << L" clickable=" << clickableCount
        << L" withText=" << textCount
        << L" travelHits=" << travelRows.size() << L"\n\n";

    out << L"=== TRAVEL-LIKE HITS (ưu tiên) ===\n";
    if (travelRows.empty()) {
        out << L"(Chưa thấy text Đại Lý/Lạc Dương/Tô Châu/... trong UI live.)\n";
    } else {
        for (std::size_t i = 0; i < travelRows.size(); ++i) EmitUiRow(out, travelRows[i], i + 1, L"TRAVEL");
    }

    out << L"\n=== CLICKABLE / TEXT CONTROLS (tối đa 220) ===\n";
    for (std::size_t i = 0; i < otherRows.size(); ++i) EmitUiRow(out, otherRows[i], i + 1, L"UI");
    if (otherRows.size() >= 220) out << L"... output đã cap 220 control phụ để không tràn shared buffer.\n";

    out << L"\nSUMMARY travelHits=" << travelRows.size()
        << L" active=" << activeCount
        << L" clickable=" << clickableCount
        << L" text=" << textCount << L"\n";
    detail = out.str();

    return travelRows.empty() ? ResultCode::GameDialogNotOpen : ResultCode::Ok;
}

bool EnsureShared() {
    if (gShared) return true;
    wchar_t mappingName[96]{};
    MappingName(GetCurrentProcessId(), mappingName, _countof(mappingName));
    gMapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, mappingName);
    if (!gMapping) return false;
    gShared = reinterpret_cast<SharedBlock*>(
        MapViewOfFile(gMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    return gShared && gShared->magic == kMagic &&
           gShared->protocolVersion == kProtocolVersion &&
           gShared->targetPid == GetCurrentProcessId();
}

void Process() {
    if (!EnsureShared()) return;
    const LONG seq = gShared->requestSeq;
    if (seq <= 0 || seq == gShared->completedSeq) return;
    if (InterlockedCompareExchange(&gShared->bridgeBusy, 1, 0) != 0) return;

    ResultCode result = ResultCode::InternalError;
    std::wstring detail;
    gShared->out0 = 0;
    gShared->out1 = 0;

    if (GetCurrentThreadId() != gShared->targetTid) {
        result = ResultCode::WrongThread;
        detail = L"Sai callback thread";
    } else {
        std::wstring why;
        if (!gApi.Load(why)) {
            result = GetModuleHandleW(L"GameAssembly.dll")
                ? ResultCode::Il2CppExportMissing
                : ResultCode::GameAssemblyMissing;
            detail = why;
        } else {
            switch (gShared->command) {
                case Command::ValidateContext:
                    result = ResultCode::Ok;
                    detail = L"PASS READ-ONLY: đúng window thread; build này không chứa command gameplay mutation.";
                    break;
                case Command::DumpNearbyObjects:
                    result = DumpNearby(detail);
                    break;
                case Command::DumpGameDialog:
                    result = DumpUiLive(detail);
                    break;
                case Command::ReadPlayerState: {
                    std::wstring player;
                    if (Leader(player)) {
                        result = ResultCode::Ok;
                        detail = player;
                    } else {
                        result = ResultCode::FieldReadFailed;
                        detail = L"Không đọc được LeaderRoleData";
                    }
                    break;
                }
                default:
                    result = ResultCode::InternalError;
                    detail = L"Command không hợp lệ";
                    break;
            }
        }
    }

    gShared->result = result;
    wcsncpy_s(gShared->detail, _countof(gShared->detail), detail.c_str(), _TRUNCATE);
    MemoryBarrier();
    InterlockedExchange(&gShared->completedSeq, seq);
    InterlockedExchange(&gShared->bridgeBusy, 0);
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlnpGetMessageHook(
    int code, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    if (code >= 0 && lParam) {
        const MSG* message = reinterpret_cast<const MSG*>(lParam);
        if (message->message == kWakeMessage) Process();
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(instance);
    if (reason == DLL_PROCESS_DETACH) {
        if (gShared) UnmapViewOfFile(gShared);
        if (gMapping) CloseHandle(gMapping);
        gShared = nullptr;
        gMapping = nullptr;
    }
    return TRUE;
}
