#ifndef UNITYRESOLVE_HPP
#define UNITYRESOLVE_HPP

// ============================== 自动检测当前环境 ==============================

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS_MODE 1
#else
#define WINDOWS_MODE 0
#endif

#if defined(__ANDROID__)
#define ANDROID_MODE 1
#else
#define ANDROID_MODE 0
#endif

#if defined(TARGET_OS_IOS)
#define IOS_MODE 1
#else
#define IOS_MODE 0
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#define LINUX_MODE 1
#else
#define LINUX_MODE 0
#endif

#if defined(__harmony__) && !defined(_HARMONYOS)
#define HARMONYOS_MODE 1
#else
#define HARMONYOS_MODE 0
#endif

// ============================== 强制设置当前执行环境 ==============================

// #define WINDOWS_MODE 0
// #define ANDROID_MODE 1 // 设置运行环境
// #define LINUX_MODE 0
// #define IOS_MODE 0
// #define HARMONYOS_MODE 0

// ============================== 导入对应环境依赖 ==============================

#if WINDOWS_MODE || LINUX_MODE || IOS_MODE
#include <format>
#include <ranges>
#include <regex>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <numbers>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "FastMemcpy_Avx.h"

#include "../../util.h"

#ifdef USE_GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#endif

#if WINDOWS_MODE
#include <windows.h>
#undef GetObject
#endif

#if WINDOWS_MODE
#ifdef _WIN64
#define UNITY_CALLING_CONVENTION __fastcall
#elif _WIN32
#define UNITY_CALLING_CONVENTION __cdecl
#endif
#elif ANDROID_MODE || LINUX_MODE || IOS_MODE || HARMONYOS_MODE
#include <dlfcn.h>
#include <locale>
#define UNITY_CALLING_CONVENTION
#endif

class UnityResolve final {
public:
    struct Assembly;
    struct Type;
    struct Class;
    struct Field;
    struct Method;
    class UnityType;

    enum class Mode : char { Il2Cpp, Mono, };

    struct Assembly final {
        void* address;
        int name;
        int file;
        std::vector<Class*, mi_stl_allocator<Class*>> classes;

        [[nodiscard]] auto Get(const util::String& strClass, const util::String& strNamespace = "*", const util::String& strParent = "*") const -> Class* {
            for (const auto pClass : classes) {
                if (strClass == name_map[pClass->name] && (strNamespace == "*" || name_map[pClass->namespaze] == strNamespace) && (strParent == "*" || name_map[pClass->parent] == strParent)) {
                    return pClass;
                }
            }
            return nullptr;
        }
    };

    struct Type final {
        void* address;
        int name;
        int size;

        // UnityType::CsType*
        [[nodiscard]] auto GetCSType() const -> void* {
            if (mode_ == Mode::Il2Cpp) {
                return Invoke<void*>("il2cpp_type_get_object", address);
            }
            return Invoke<void*>("mono_type_get_object", pDomain, address);
        }
    };

    struct Class final {
        void* address;
        int name;
        int parent;
        int namespaze;
        std::vector<Field, mi_stl_allocator<Field>> fields;
        std::vector<Method, mi_stl_allocator<Method>> methods;
        void* objType;

        template<typename RType>
        auto Get(const util::String& name, const std::vector<util::String>& args = {}) -> RType* {
            if constexpr (std::is_same_v<RType, Field>) {
                for (auto& pField : fields) {
                    if (name_map[pField.name] == name) {
                        return static_cast<RType*>(&pField);
                    }
                }
                LOG_WARNING << "字段：" << name.data() << " 未找到!";
            }
            if constexpr (std::is_same_v<RType, Method>) {
                for (auto& pMethod : methods) {
                    if (name_map[pMethod.name] == name) {
                        if (pMethod.args.empty() && args.empty()) {
                            return static_cast<RType*>(&pMethod);
                        }
                        if (pMethod.args.size() == args.size()) {
                            size_t index{0};
                            for (size_t i{0}; const auto& typeName : args) {
                                if (typeName == "*" || typeName.empty() ? true : name_map[pMethod.args[i++]->pType->name] == typeName) {
                                    index++;
                                }
                            }
                            if (index == pMethod.args.size()) {
                                return static_cast<RType*>(&pMethod);
                            }
                        }
                    }
                }

                for (auto& pMethod : methods) {
                    if (name_map[pMethod.name] == name) {
                        return static_cast<RType*>(&pMethod);
                    }
                }
                LOG_WARNING << "方法：" << name.data() << " 未找到!";
            }
            return nullptr;
        }

        template<typename RType>
        auto GetValue(void* obj, const util::String& name) -> RType {
            return *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + Get<Field>(name)->offset);
        }

        template<typename RType>
        auto GetValue(void* obj, unsigned int offset) -> RType {
            return *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + offset);
        }

        template<typename RType>
        auto SetValue(void* obj, const util::String& name, RType value) -> void {
            *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + Get<Field>(name)->offset) = value;
        }

        template<typename RType>
        auto SetValue(void* obj, unsigned int offset, RType value) -> RType {
            *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + offset) = value;
        }

        // UnityType::CsType*
        [[nodiscard]] auto GetType() -> void* {
            if (objType) {
                return objType;
            }
            if (mode_ == Mode::Il2Cpp) {
                const auto pUType = Invoke<void*, void*>("il2cpp_class_get_type", address);
                objType = Invoke<void*>("il2cpp_type_get_object", pUType);
                return objType;
            }
            const auto pUType = Invoke<void*, void*>("mono_class_get_type", address);
            objType = Invoke<void*>("mono_type_get_object", pDomain, pUType);
            return objType;
        }

        /**
         * \brief 获取类所有实例
         * \tparam T 返回数组类型
         * \param type 类
         * \return 返回实例指针数组
         */
        template<typename T>
        auto FindObjectsByType() -> std::vector<T, mi_stl_allocator<T>> {
            static Method* pMethod;

            if (!pMethod) {
                pMethod = UnityResolve::Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("FindObjectsOfType", {"System.Type"});
            }
            if (!objType) {
                objType = this->GetType();
            }

            if (pMethod && objType) {
                if (auto array = pMethod->Invoke<UnityType::Array<T>*>(objType)) {
                    return array->ToVector();
                }
            }

            return {};
        }

        template<typename T>
        auto New() -> T* {
            if (mode_ == Mode::Il2Cpp) {
                return Invoke<T*, void*>("il2cpp_object_new", address);
            }
            return Invoke<T*, void*, void*>("mono_object_new", pDomain, address);
        }
    };

    struct Field final {
        void* address;
        int name;
        Type* type;
        Class* klass;
        std::int32_t offset; // If offset is -1, then it's thread static
        bool static_field;
        void* vTable;

        template<typename T>
        auto SetStaticValue(T* value) const -> void {
            if (!static_field) {
                return;
            }
            if (mode_ == Mode::Il2Cpp) {
                return Invoke<void, void*, T*>("il2cpp_field_static_set_value", address, value);
            }
            const auto VTable = Invoke<void*>("mono_class_vtable", pDomain, klass->address);
            return Invoke<void, void*, void*, T*>("mono_field_static_set_value", VTable, address, value);
        }

        template<typename T>
        auto GetStaticValue(T* value) const -> void {
            if (!static_field) {
                return;
            }
            if (mode_ == Mode::Il2Cpp) {
                return Invoke<void, void*, T*>("il2cpp_field_static_get_value", address, value);
            }
            const auto VTable = Invoke<void*>("mono_class_vtable", pDomain, klass->address);
            return Invoke<void, void*, void*, T*>("mono_field_static_get_value", VTable, address, value);
        }

        template<typename C, typename T>
        struct Variable {
        public:
            std::int32_t offset{ 0 };

            auto Init(const Field* field) -> void {
                offset = field->offset;
            }

            auto Get(C* obj) -> T {
                return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset);
            }

            auto Set(C* obj, T value) -> void {
                *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset) = value;
            }

            auto operator[](C* obj) -> T& {
                return *reinterpret_cast<T*>(offset + reinterpret_cast<std::uintptr_t>(obj));
            }
        };
    };

    template<typename Return, typename... Args>
    using MethodPointer = Return(UNITY_CALLING_CONVENTION*)(Args...);

    struct Method final {
        void* address = nullptr;
        int name;
        Class* klass = nullptr;
        Type* return_type = nullptr;
        std::int32_t flags = 0;
        bool static_function = false;
        void* function = nullptr;

        struct Arg {
            int name;
            Type* pType = nullptr;
        };

        std::vector<Arg*, mi_stl_allocator<Arg*>> args;

        enum class Mode { Mono, Il2Cpp };

        static inline auto mode_ = Mode::Mono;

        template<typename Return, typename... Args>
        auto Invoke(Args... args) -> Return {
            Compile();
            if (function) {
                using FuncType = Return(UNITY_CALLING_CONVENTION*)(Args...);
                return reinterpret_cast<FuncType>(function)(std::forward<Args>(args)...);
            }
            if (!std::is_void_v<Return>) {
                return Return{}; // Return default constructed if not void
            }
        }

        auto Compile() -> void {
            if (address && !function && mode_ == Mode::Mono) {
                function = UnityResolve::Invoke<void*>("mono_compile_method", address);
            }
        }

        template<typename Return, typename... Args>
        auto Cast() -> MethodPointer<Return, Args...> {
            Compile();
            if (function) {
                return reinterpret_cast<MethodPointer<Return, Args...>>(function);
            }
            return nullptr;
        }

        template<typename Return, typename... Args>
        auto Cast(MethodPointer<Return, Args...>& ptr) -> MethodPointer<Return, Args...> {
            Compile();
            if (function) {
                ptr = reinterpret_cast<MethodPointer<Return, Args...>>(function);
                return ptr;
            }
            return nullptr;
        }

        template<typename Return, typename... Args>
        auto Cast(std::function<Return(Args...)>& ptr) -> std::function<Return(Args...)> {
            Compile();
            if (function) {
                auto rawPtr = reinterpret_cast<MethodPointer<Return, Args...>>(function);
                ptr = [rawPtr](Args... args) {
                    return rawPtr(std::forward<Args>(args)...);
                };
                return ptr;
            }
            return nullptr;
        }

        template<typename T>
        auto Unbox(void* obj) -> T {
            if (mode_ == Mode::Il2Cpp) {
                return static_cast<T>(Invoke<void*>("il2cpp_object_unbox", obj));
            }
            return static_cast<T>(Invoke<void*>("mono_object_unbox", obj));
        }
    };


    class AssemblyLoad {
    public:
        AssemblyLoad(const util::String& path, util::String namespaze = "", util::String className = "", util::String desc = "") {
            if (mode_ == Mode::Mono) {
                assembly = Invoke<void*>("mono_domain_assembly_open", pDomain, path.data());
                image = Invoke<void*>("mono_assembly_get_image", assembly);
                if (namespaze.empty() || className.empty() || desc.empty()) {
                    return;
                }
                klass = Invoke<void*>("mono_class_from_name", image, namespaze.data(), className.data());
                const auto entry_point_method_desc = Invoke<void*>("mono_method_desc_new", desc.data(), true);
                method = Invoke<void*>("mono_method_desc_search_in_class", entry_point_method_desc, klass);
                Invoke<void>("mono_method_desc_free", entry_point_method_desc);
                Invoke<void*>("mono_runtime_invoke", method, nullptr, nullptr, nullptr);
            }
        }

        void* assembly;
        void* image;
        void* klass;
        void* method;
    };

    static auto ThreadAttach() -> void {
        if (mode_ == Mode::Il2Cpp) {
            Invoke<void*>("il2cpp_thread_attach", pDomain);
        } else {
            Invoke<void*>("mono_thread_attach", pDomain);
            Invoke<void*>("mono_jit_thread_attach", pDomain);
        }
    }

    static auto ThreadDetach() -> void {
        if (mode_ == Mode::Il2Cpp) {
            Invoke<void*>("il2cpp_thread_detach", pDomain);
        } else {
            Invoke<void*>("mono_thread_detach", pDomain);
            Invoke<void*>("mono_jit_thread_detach", pDomain);
        }
    }

    static auto Init(void* hmodule, const Mode mode = Mode::Mono) -> void {
        mode_ = mode;
        hmodule_ = hmodule;

        if (mode_ == Mode::Il2Cpp) {
            pDomain = Invoke<void*>("il2cpp_domain_get");
            while (!Invoke<bool>("il2cpp_is_vm_thread", nullptr)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            Invoke<void*>("il2cpp_thread_attach", pDomain);
            ForeachAssembly();
        } else {
            pDomain = Invoke<void*>("mono_get_root_domain");
            Invoke<void*>("mono_thread_attach", pDomain);
            Invoke<void*>("mono_jit_thread_attach", pDomain);

            ForeachAssembly();
        }
    }

    template<typename Return, typename... Args>
    static auto Invoke(const util::String& funcName, Args... args) -> Return {
#if WINDOWS_MODE
        if (!address_.contains(funcName) || !address_[funcName]) {
            address_[funcName] = static_cast<void*>(GetProcAddress(static_cast<HMODULE>(hmodule_), funcName.c_str()));
        }
#elif ANDROID_MODE || LINUX_MODE || IOS_MODE || HARMONYOS_MODE
        if (address_.find(funcName) == address_.end() || !address_[funcName]) {
            address_[funcName] = dlsym(hmodule_, funcName.c_str());
        }
#endif

        if (address_[funcName] != nullptr) {
            try {
                return reinterpret_cast<Return(UNITY_CALLING_CONVENTION*)(Args...)>(address_[funcName])(args...);
            } catch (...) {
                return Return();
            }
        }
        return Return();
    }

    inline static std::vector<Assembly*> assembly;

    // 全局字符串
    inline static util::Map<int, util::String> name_map;

    static auto Get(const util::String& strAssembly) -> Assembly* {
        for (const auto pAssembly : assembly) {
            if (name_map[pAssembly->name] == strAssembly) {
                return pAssembly;
            }
        }
        return nullptr;
    }

private:
    // 去除重复字符串使用
    inline static util::Map<util::String, int> temp;

    static auto ForeachAssembly() -> void {

        if (mode_ == Mode::Il2Cpp) {
            size_t assemblyCount = 0;
            const auto assemblies = Invoke<void**>("il2cpp_domain_get_assemblies", pDomain, &assemblyCount);
            for (size_t i = 0; i < assemblyCount; ++i) {
                const auto assemblyPtr = assemblies[i];
                if (!assemblyPtr) {
                    continue;
                }

                auto assembly = new Assembly{.address = assemblyPtr};
                auto image = Invoke<void*>("il2cpp_assembly_get_image", assemblyPtr);

                util::String name_file = Invoke<const char*>("il2cpp_image_get_filename", image);
                util::String name_ = Invoke<const char*>("il2cpp_image_get_name", image);

                if (name_ == "Assembly-CSharp.dll") {
                    if (!temp.contains(name_file)) {
                        temp[name_file] = temp.size() + 1;
                    }

                    if (!temp.contains(name_)) {
                        temp[name_] = temp.size() + 1;
                    }

                    assembly->file = temp[name_file];
                    assembly->name = temp[name_];
                    UnityResolve::assembly.push_back(assembly);
                    ForeachClass(assembly, image);
                    continue;
                }
                if (name_ == "mscorlib.dll") {
                    if (!temp.contains(name_file)) {
                        temp[name_file] = temp.size() + 1;
                    }

                    if (!temp.contains(name_)) {
                        temp[name_] = temp.size() + 1;
                    }

                    assembly->file = temp[name_file];
                    assembly->name = temp[name_];
                    UnityResolve::assembly.push_back(assembly);
                    ForeachClass(assembly, image);
                    continue;
                }
                if (name_ == "UnityEngine.CoreModule.dll") {
                    if (!temp.contains(name_file)) {
                        temp[name_file] = temp.size() + 1;
                    }

                    if (!temp.contains(name_)) {
                        temp[name_] = temp.size() + 1;
                    }

                    assembly->file = temp[name_file];
                    assembly->name = temp[name_];
                    UnityResolve::assembly.push_back(assembly);
                    ForeachClass(assembly, image);
                    continue;
                }
                if (name_ == "UnityEngine.AnimationModule.dll") {
                    if (!temp.contains(name_file)) {
                        temp[name_file] = temp.size() + 1;
                    }

                    if (!temp.contains(name_)) {
                        temp[name_] = temp.size() + 1;
                    }

                    assembly->file = temp[name_file];
                    assembly->name = temp[name_];
                    UnityResolve::assembly.push_back(assembly);
                    ForeachClass(assembly, image);
                    continue;
                }
                if (name_ == "UnityEngine.PhysicsModule.dll") {
                    if (!temp.contains(name_file)) {
                        temp[name_file] = temp.size() + 1;
                    }

                    if (!temp.contains(name_)) {
                        temp[name_] = temp.size() + 1;
                    }

                    assembly->file = temp[name_file];
                    assembly->name = temp[name_];
                    UnityResolve::assembly.push_back(assembly);
                    ForeachClass(assembly, image);
                    continue;
                }

                delete assembly;
            }
        }

        for (auto& [name, value] : temp) {
            name_map[value] = name;
        }
        temp.clear();
    }

    static auto ForeachClass(Assembly* assembly, void* image) -> void {
        if (mode_ == Mode::Il2Cpp) {
            int classCount = Invoke<int>("il2cpp_image_get_class_count", image);
            for (int i = 0; i < classCount; ++i) {
                auto classPtr = Invoke<void*>("il2cpp_image_get_class", image, i);
                if (!classPtr) {
                    continue;
                }

                auto klass = new Class{.address = classPtr};
                util::String a = Invoke<const char*>("il2cpp_class_get_name", classPtr);
                if (!temp.contains(a)) {
                    temp[a] = temp.size() + 1;
                }
                klass->name = temp[a];
                if (auto parent = Invoke<void*>("il2cpp_class_get_parent", classPtr)) {
                    util::String a = Invoke<const char*>("il2cpp_class_get_name", parent);
                    if (!temp.contains(a)) {
                        temp[a] = temp.size() + 1;
                    }
                    klass->parent = temp[a];
                }
                util::String b = Invoke<const char*>("il2cpp_class_get_namespace", classPtr);
                if (!temp.contains(b)) {
                    temp[b] = temp.size() + 1;
                }
                klass->namespaze = temp[b];
                assembly->classes.push_back(klass);

                ForeachFields(klass, classPtr);
                ForeachMethod(klass, classPtr);

                void* iface = nullptr;
                void* iter = nullptr;
                while ((iface = Invoke<void*>("il2cpp_class_get_interfaces", classPtr, &iter))) {
                    ForeachFields(klass, iface);
                    ForeachMethod(klass, iface);
                }
            }
        }
    }

    static auto ForeachFields(Class* klass, void* classPtr) -> void {
        if (mode_ == Mode::Il2Cpp) {
            void* iter = nullptr;
            void* field = nullptr;
            while ((field = Invoke<void*>("il2cpp_class_get_fields", classPtr, &iter))) {
                util::String a = Invoke<const char*>("il2cpp_field_get_name", field);
                if (!temp.contains(a)) {
                    temp[a] = temp.size() + 1;
                }
                auto type = new Type{.address = Invoke<void*>("il2cpp_field_get_type", field)};
                auto fieldObj = Field{
                    .address = field,
                    .name = temp[a],
                    .type = type,
                    .klass = klass,
                    .offset = Invoke<int>("il2cpp_field_get_offset", field),
                    .static_field = false,
                    .vTable = nullptr
                };
                fieldObj.static_field = fieldObj.offset <= 0;
                const auto typeName = Invoke<char*>("il2cpp_type_get_name", type->address);
                if (!temp.contains(typeName)) {
                    temp[typeName] = temp.size() + 1;
                }

                type->name = temp[typeName];
                type->size = -1;
                Invoke<void>("il2cpp_free", typeName);
                klass->fields.push_back(fieldObj);
            }
        }
    }


    static auto ForeachMethod(Class* klass, void* pKlass) -> void {
        // 遍历方法
        if (mode_ == Mode::Il2Cpp) {
            void* iter = nullptr;
            void* method = nullptr;

            while ((method = Invoke<void*>("il2cpp_class_get_methods", pKlass, &iter))) {
                int fFlags{};
                auto pMethod = Method{};
                pMethod.address = method;
                util::String a = Invoke<const char*>("il2cpp_method_get_name", method);
                if (!temp.contains(a)) {
                    temp[a] = temp.size() + 1;
                }
                pMethod.name = temp[a];
                pMethod.klass = klass;

                auto returnType = new Type{};
                returnType->address = Invoke<void*>("il2cpp_method_get_return_type", method);
                auto returnTypeName = Invoke<char*>("il2cpp_type_get_name", returnType->address);
                if (!temp.contains(returnTypeName)) {
                    temp[returnTypeName] = temp.size() + 1;
                }
                returnType->name = temp[returnTypeName];
                Invoke<void>("il2cpp_free", returnTypeName);
                returnType->size = -1;

                pMethod.return_type = returnType;
                pMethod.flags = Invoke<int>("il2cpp_method_get_flags", method, &fFlags);
                pMethod.static_function = (pMethod.flags & 0x10) != 0;
                pMethod.function = *static_cast<void**>(method);

                int argCount = Invoke<int>("il2cpp_method_get_param_count", method);
                for (int i = 0; i < argCount; ++i) {
                    auto arg = new Method::Arg();
                    util::String a = Invoke<const char*>("il2cpp_method_get_param_name", method, i);
                    if (!temp.contains(a)) {
                        temp[a] = temp.size() + 1;
                    }
                    arg->name = temp[a];

                    auto argType = new Type();
                    argType->address = Invoke<void*>("il2cpp_method_get_param", method, i);
                    auto argTypeName = Invoke<char*>("il2cpp_type_get_name", argType->address);
                    if (!temp.contains(argTypeName)) {
                        temp[argTypeName] = temp.size() + 1;
                    }
                    argType->name = temp[argTypeName];
                    Invoke<void>("il2cpp_free", argTypeName);
                    argType->size = -1;

                    arg->pType = argType;
                    pMethod.args.emplace_back(arg);
                }

                klass->methods.push_back(pMethod);
            }
        }
    }

public:
    class UnityType final {
    public:
        using IntPtr = std::uintptr_t;
        using Int32 = std::int32_t;
        using Int64 = std::int64_t;
        using Char = wchar_t;
        using Int16 = std::int16_t;
        using Byte = std::uint8_t;
#ifndef USE_GLM
        struct Vector3;
        struct Vector4;
        struct Vector2;
        struct Quaternion;
        struct Matrix4x4;
#else
        using Vector3 = glm::vec3;
        using Vector2 = glm::vec2;
        using Vector4 = glm::vec4;
        using Quaternion = glm::quat;
        using Matrix4x4 = glm::mat4x4;
#endif
        struct Camera;
        struct Transform;
        struct Component;
        struct UnityObject;
        struct LayerMask;
        struct Rigidbody;
        struct Physics;
        struct GameObject;
        struct Collider;
        struct Bounds;
        struct Plane;
        struct Ray;
        struct Rect;
        struct Color;
        template<typename T>
        struct Array;
        struct String;
        struct Object;
        template<typename T>
        struct List;
        template<typename TKey, typename TValue>
        struct Dictionary;
        struct Behaviour;
        struct MonoBehaviour;
        struct CsType;
        struct Mesh;
        struct Renderer;
        struct Animator;
        struct CapsuleCollider;
        struct BoxCollider;
        struct FieldInfo;
        struct MethodInfo;
        struct PropertyInfo;
        struct Assembly;
        struct EventInfo;
        struct MemberInfo;
        struct Time;
        struct RaycastHit;

#ifndef USE_GLM
        struct Vector3 {
            float x, y, z;

            Vector3() { x = y = z = 0.f; }

            Vector3(const float f1, const float f2, const float f3) {
                x = f1;
                y = f2;
                z = f3;
            }

            [[nodiscard]] auto Length() const -> float { return x * x + y * y + z * z; }

            [[nodiscard]] auto Dot(const Vector3 b) const -> float { return x * b.x + y * b.y + z * b.z; }

            [[nodiscard]] auto Normalize() const -> Vector3 {
                if (const auto len = Length(); len > 0)
                    return Vector3(x / len, y / len, z / len);
                return Vector3(x, y, z);
            }

            auto ToVectors(Vector3* m_pForward, Vector3* m_pRight, Vector3* m_pUp) const -> void {
                auto m_fDeg2Rad = std::numbers::pi_v<float> / 180.F;

                const auto m_fSinX = sinf(x * m_fDeg2Rad);
                const auto m_fCosX = cosf(x * m_fDeg2Rad);

                const auto m_fSinY = sinf(y * m_fDeg2Rad);
                const auto m_fCosY = cosf(y * m_fDeg2Rad);

                const auto m_fSinZ = sinf(z * m_fDeg2Rad);
                const auto m_fCosZ = cosf(z * m_fDeg2Rad);

                if (m_pForward) {
                    m_pForward->x = m_fCosX * m_fCosY;
                    m_pForward->y = -m_fSinX;
                    m_pForward->z = m_fCosX * m_fSinY;
                }

                if (m_pRight) {
                    m_pRight->x = -1.f * m_fSinZ * m_fSinX * m_fCosY + -1.f * m_fCosZ * -m_fSinY;
                    m_pRight->y = -1.f * m_fSinZ * m_fCosX;
                    m_pRight->z = -1.f * m_fSinZ * m_fSinX * m_fSinY + -1.f * m_fCosZ * m_fCosY;
                }

                if (m_pUp) {
                    m_pUp->x = m_fCosZ * m_fSinX * m_fCosY + -m_fSinZ * -m_fSinY;
                    m_pUp->y = m_fCosZ * m_fCosX;
                    m_pUp->z = m_fCosZ * m_fSinX * m_fSinY + -m_fSinZ * m_fCosY;
                }
            }

            [[nodiscard]] auto Distance(Vector3& event) const -> float {
                const auto dx = this->x - event.x;
                const auto dy = this->y - event.y;
                const auto dz = this->z - event.z;
                return std::sqrt(dx * dx + dy * dy + dz * dz);
            }

            auto operator*(const float x) -> Vector3 {
                this->x *= x;
                this->y *= x;
                this->z *= x;
                return *this;
            }

            auto operator-(const float x) -> Vector3 {
                this->x -= x;
                this->y -= x;
                this->z -= x;
                return *this;
            }

            auto operator+(const float x) -> Vector3 {
                this->x += x;
                this->y += x;
                this->z += x;
                return *this;
            }

            auto operator/(const float x) -> Vector3 {
                this->x /= x;
                this->y /= x;
                this->z /= x;
                return *this;
            }

            auto operator*(const Vector3 x) -> Vector3 {
                this->x *= x.x;
                this->y *= x.y;
                this->z *= x.z;
                return *this;
            }

            auto operator-(const Vector3 x) -> Vector3 {
                this->x -= x.x;
                this->y -= x.y;
                this->z -= x.z;
                return *this;
            }

            auto operator+(const Vector3 x) -> Vector3 {
                this->x += x.x;
                this->y += x.y;
                this->z += x.z;
                return *this;
            }

            auto operator/(const Vector3 x) -> Vector3 {
                this->x /= x.x;
                this->y /= x.y;
                this->z /= x.z;
                return *this;
            }

            auto operator==(const Vector3 x) const -> bool { return this->x == x.x && this->y == x.y && this->z == x.z; }
        };
#endif

#ifndef USE_GLM
        struct Vector2 {
            float x, y;

            Vector2() { x = y = 0.f; }

            Vector2(const float f1, const float f2) {
                x = f1;
                y = f2;
            }

            [[nodiscard]] auto Distance(const Vector2& event) const -> float {
                const auto dx = this->x - event.x;
                const auto dy = this->y - event.y;
                return std::sqrt(dx * dx + dy * dy);
            }

            auto operator*(const float x) -> Vector2 {
                this->x *= x;
                this->y *= x;
                return *this;
            }

            auto operator/(const float x) -> Vector2 {
                this->x /= x;
                this->y /= x;
                return *this;
            }

            auto operator+(const float x) -> Vector2 {
                this->x += x;
                this->y += x;
                return *this;
            }

            auto operator-(const float x) -> Vector2 {
                this->x -= x;
                this->y -= x;
                return *this;
            }

            auto operator*(const Vector2 x) -> Vector2 {
                this->x *= x.x;
                this->y *= x.y;
                return *this;
            }

            auto operator-(const Vector2 x) -> Vector2 {
                this->x -= x.x;
                this->y -= x.y;
                return *this;
            }

            auto operator+(const Vector2 x) -> Vector2 {
                this->x += x.x;
                this->y += x.y;
                return *this;
            }

            auto operator/(const Vector2 x) -> Vector2 {
                this->x /= x.x;
                this->y /= x.y;
                return *this;
            }

            auto operator==(const Vector2 x) const -> bool { return this->x == x.x && this->y == x.y; }
        };
#endif

#ifndef USE_GLM
        struct Vector4 {
            float x, y, z, w;

            Vector4() { x = y = z = w = 0.F; }

            Vector4(const float f1, const float f2, const float f3, const float f4) {
                x = f1;
                y = f2;
                z = f3;
                w = f4;
            }

            auto operator*(const float x) -> Vector4 {
                this->x *= x;
                this->y *= x;
                this->z *= x;
                this->w *= x;
                return *this;
            }

            auto operator-(const float x) -> Vector4 {
                this->x -= x;
                this->y -= x;
                this->z -= x;
                this->w -= x;
                return *this;
            }

            auto operator+(const float x) -> Vector4 {
                this->x += x;
                this->y += x;
                this->z += x;
                this->w += x;
                return *this;
            }

            auto operator/(const float x) -> Vector4 {
                this->x /= x;
                this->y /= x;
                this->z /= x;
                this->w /= x;
                return *this;
            }

            auto operator*(const Vector4 x) -> Vector4 {
                this->x *= x.x;
                this->y *= x.y;
                this->z *= x.z;
                this->w *= x.w;
                return *this;
            }

            auto operator-(const Vector4 x) -> Vector4 {
                this->x -= x.x;
                this->y -= x.y;
                this->z -= x.z;
                this->w -= x.w;
                return *this;
            }

            auto operator+(const Vector4 x) -> Vector4 {
                this->x += x.x;
                this->y += x.y;
                this->z += x.z;
                this->w += x.w;
                return *this;
            }

            auto operator/(const Vector4 x) -> Vector4 {
                this->x /= x.x;
                this->y /= x.y;
                this->z /= x.z;
                this->w /= x.w;
                return *this;
            }

            auto operator==(const Vector4 x) const -> bool { return this->x == x.x && this->y == x.y && this->z == x.z && this->w == x.w; }
        };
#endif

#ifndef USE_GLM
        struct Quaternion {
            float x, y, z, w;

            Quaternion() { x = y = z = w = 0.F; }

            Quaternion(const float f1, const float f2, const float f3, const float f4) {
                x = f1;
                y = f2;
                z = f3;
                w = f4;
            }

            auto Euler(float m_fX, float m_fY, float m_fZ) -> Quaternion {
                auto m_fDeg2Rad = std::numbers::pi_v<float> / 180.F;

                m_fX = m_fX * m_fDeg2Rad * 0.5F;
                m_fY = m_fY * m_fDeg2Rad * 0.5F;
                m_fZ = m_fZ * m_fDeg2Rad * 0.5F;

                const auto m_fSinX = sinf(m_fX);
                const auto m_fCosX = cosf(m_fX);

                const auto m_fSinY = sinf(m_fY);
                const auto m_fCosY = cosf(m_fY);

                const auto m_fSinZ = sinf(m_fZ);
                const auto m_fCosZ = cosf(m_fZ);

                x = m_fCosY * m_fSinX * m_fCosZ + m_fSinY * m_fCosX * m_fSinZ;
                y = m_fSinY * m_fCosX * m_fCosZ - m_fCosY * m_fSinX * m_fSinZ;
                z = m_fCosY * m_fCosX * m_fSinZ - m_fSinY * m_fSinX * m_fCosZ;
                w = m_fCosY * m_fCosX * m_fCosZ + m_fSinY * m_fSinX * m_fSinZ;

                return *this;
            }

            auto Euler(const Vector3& m_vRot) -> Quaternion { return Euler(m_vRot.x, m_vRot.y, m_vRot.z); }

            [[nodiscard]] auto ToEuler() const -> Vector3 {
                Vector3 m_vEuler;

                const auto m_fDist = (x * x) + (y * y) + (z * z) + (w * w);

                if (const auto m_fTest = x * w - y * z; m_fTest > 0.4995F * m_fDist) {
                    m_vEuler.x = std::numbers::pi_v<float> *0.5F;
                    m_vEuler.y = 2.F * atan2f(y, x);
                    m_vEuler.z = 0.F;
                }
                else if (m_fTest < -0.4995F * m_fDist) {
                    m_vEuler.x = std::numbers::pi_v<float> *-0.5F;
                    m_vEuler.y = -2.F * atan2f(y, x);
                    m_vEuler.z = 0.F;
                }
                else {
                    m_vEuler.x = asinf(2.F * (w * x - y * z));
                    m_vEuler.y = atan2f(2.F * w * y + 2.F * z * x, 1.F - 2.F * (x * x + y * y));
                    m_vEuler.z = atan2f(2.F * w * z + 2.F * x * y, 1.F - 2.F * (z * z + x * x));
                }

                auto m_fRad2Deg = 180.F / std::numbers::pi_v<float>;
                m_vEuler.x *= m_fRad2Deg;
                m_vEuler.y *= m_fRad2Deg;
                m_vEuler.z *= m_fRad2Deg;

                return m_vEuler;
            }

            static auto LookRotation(const Vector3& forward) -> Quaternion {
                static Method* method;

                if (!method)
                    method = Get("UnityEngine.CoreModule.dll")->Get("Quaternion")->Get<Method>("LookRotation", { "UnityEngine.Vector3" });
                if (method)
                    return method->Invoke<Quaternion, Vector3>(forward);
                return {};
            }

            auto operator*(const float x) -> Quaternion {
                this->x *= x;
                this->y *= x;
                this->z *= x;
                this->w *= x;
                return *this;
            }

            auto operator-(const float x) -> Quaternion {
                this->x -= x;
                this->y -= x;
                this->z -= x;
                this->w -= x;
                return *this;
            }

            auto operator+(const float x) -> Quaternion {
                this->x += x;
                this->y += x;
                this->z += x;
                this->w += x;
                return *this;
            }

            auto operator/(const float x) -> Quaternion {
                this->x /= x;
                this->y /= x;
                this->z /= x;
                this->w /= x;
                return *this;
            }

            auto operator*(const Quaternion x) -> Quaternion {
                this->x *= x.x;
                this->y *= x.y;
                this->z *= x.z;
                this->w *= x.w;
                return *this;
            }

            auto operator-(const Quaternion x) -> Quaternion {
                this->x -= x.x;
                this->y -= x.y;
                this->z -= x.z;
                this->w -= x.w;
                return *this;
            }

            auto operator+(const Quaternion x) -> Quaternion {
                this->x += x.x;
                this->y += x.y;
                this->z += x.z;
                this->w += x.w;
                return *this;
            }

            auto operator/(const Quaternion x) -> Quaternion {
                this->x /= x.x;
                this->y /= x.y;
                this->z /= x.z;
                this->w /= x.w;
                return *this;
            }

            auto operator==(const Quaternion x) const -> bool { return this->x == x.x && this->y == x.y && this->z == x.z && this->w == x.w; }
        };
#endif

        struct Bounds {
            Vector3 m_vCenter;
            Vector3 m_vExtents;
        };

        struct Plane {
            Vector3 m_vNormal;
            float fDistance;
        };

        struct Ray {
            Vector3 m_vOrigin;
            Vector3 m_vDirection;
        };

        struct RaycastHit {
            glm::vec3 m_Point;
            glm::vec3 m_Normal;
            uint32_t m_FaceID;
            float m_Distance;
            glm::vec2 m_UV;
            uint32_t m_Collider;

            auto get_collider() -> Collider* {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("RaycastHit")->Get<Method>("get_collider");
                return method->Invoke<Collider*>(this);
            }

            auto get_rigidbody() -> Rigidbody* {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("RaycastHit")->Get<Method>("get_rigidbody");
                return method->Invoke<Rigidbody*>(this);
            }
        };

        struct Rect {
            float fX, fY;
            float fWidth, fHeight;

            Rect() {
                fX = fY = fWidth = fHeight = 0.f;
            }

            Rect(const float f1, const float f2, const float f3, const float f4) {
                fX = f1;
                fY = f2;
                fWidth = f3;
                fHeight = f4;
            }
        };

        struct Color {
            float r, g, b, a;

            Color() {
                r = g = b = a = 0.f;
            }

            explicit Color(const float fRed = 0.f, const float fGreen = 0.f, const float fBlue = 0.f, const float fAlpha = 1.f) {
                r = fRed;
                g = fGreen;
                b = fBlue;
                a = fAlpha;
            }
        };

#ifndef USE_GLM
        struct Matrix4x4 {
            float m[4][4] = { {0} };

            Matrix4x4() = default;

            auto operator[](const int i) -> float* { return m[i]; }
        };
#endif

        struct Object {
            union {
                void* klass{nullptr};
                void* vtable;
            } Il2CppClass;

            struct MonitorData* monitor{nullptr};

            auto GetType() -> CsType* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Object", "System")->Get<Method>("GetType");
                }
                if (method) {
                    return method->Invoke<CsType*>(this);
                }
                return nullptr;
            }

            auto ToString() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Object", "System")->Get<Method>("ToString");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            auto GetHashCode() -> int {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Object", "System")->Get<Method>("GetHashCode");
                }
                if (method) {
                    return method->Invoke<int>(this);
                }
                return 0;
            }
        };

        enum class BindingFlags : uint32_t {
            Default              = 0,
            IgnoreCase           = 1,
            DeclaredOnly         = 2,
            Instance             = 4,
            Static               = 8,
            Public               = 16,
            NonPublic            = 32,
            FlattenHierarchy     = 64,
            InvokeMethod         = 256,
            CreateInstance       = 512,
            GetField             = 1024,
            SetField             = 2048,
            GetProperty          = 4096,
            SetProperty          = 8192,
            PutDispProperty      = 16384,
            PutRefDispProperty   = 32768,
            ExactBinding         = 65536,
            SuppressChangeType   = 131072,
            OptionalParamBinding = 262144,
            IgnoreReturn         = 16777216,
        };

        enum class FieldAttributes : uint32_t {
            FieldAccessMask = 7,
            PrivateScope    = 0,
            Private         = 1,
            FamANDAssem     = 2,
            Assembly        = 3,
            Family          = 4,
            FamORAssem      = 5,
            Public          = 6,
            Static          = 16,
            InitOnly        = 32,
            Literal         = 64,
            NotSerialized   = 128,
            HasFieldRVA     = 256,
            SpecialName     = 512,
            RTSpecialName   = 1024,
            HasFieldMarshal = 4096,
            PinvokeImpl     = 8192,
            HasDefault      = 32768,
            ReservedMask    = 38144
        };

        enum class MemberTypes : uint32_t {
            Constructor = 1,
            Event       = 2,
            Field       = 4,
            Method      = 8,
            Property    = 16,
            TypeInfo    = 32,
            Custom      = 64,
            NestedType  = 128,
            All         = 191
        };

        struct MemberInfo {};

        struct FieldInfo : MemberInfo {
            auto GetIsInitOnly() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsInitOnly");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsLiteral() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsLiteral");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsNotSerialized() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsNotSerialized");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsStatic() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsStatic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsFamily() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsFamily");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsPrivate() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsPrivate");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsPublic() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_IsPublic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetAttributes() -> FieldAttributes {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_Attributes");
                }
                if (method) {
                    return method->Invoke<FieldAttributes>(this);
                }
                return {};
            }

            auto GetMemberType() -> MemberTypes {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("get_MemberType");
                }
                if (method) {
                    return method->Invoke<MemberTypes>(this);
                }
                return {};
            }

            auto GetFieldOffset() -> int {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("GetFieldOffset");
                }
                if (method) {
                    return method->Invoke<int>(this);
                }
                return {};
            }

            template<typename T>
            auto GetValue(Object* object) -> T {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("GetValue");
                }
                if (method) {
                    return method->Invoke<T>(this, object);
                }
                return T();
            }

            template<typename T>
            auto SetValue(Object* object, T value) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("FieldInfo", "System.Reflection", "MemberInfo")->Get<Method>("SetValue", {"System.Object", "System.Object"});
                }
                if (method) {
                    return method->Invoke<T>(this, object, value);
                }
            }
        };

        struct CsType {
            auto FormatTypeName() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("FormatTypeName");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            auto GetFullName() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_FullName");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            auto GetNamespace() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_Namespace");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            auto GetIsSerializable() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsSerializable");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetContainsGenericParameters() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_ContainsGenericParameters");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsVisible() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsVisible");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsNested() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsNested");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsArray() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsArray");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsByRef() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsByRef");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsPointer() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsPointer");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsConstructedGenericType() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsConstructedGenericType");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsGenericParameter() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsGenericParameter");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsGenericMethodParameter() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsGenericMethodParameter");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsGenericType() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsGenericType");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsGenericTypeDefinition() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsGenericTypeDefinition");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsSZArray() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsSZArray");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsVariableBoundArray() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsVariableBoundArray");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetHasElementType() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_HasElementType");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsAbstract() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsAbstract");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsSealed() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsSealed");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsClass() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsClass");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsNestedAssembly() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsNestedAssembly");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsNestedPublic() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsNestedPublic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsNotPublic() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsNotPublic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsPublic() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsPublic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsExplicitLayout() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsExplicitLayout");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsCOMObject() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsCOMObject");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsContextful() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsContextful");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsCollectible() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsCollectible");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsEnum() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsEnum");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsMarshalByRef() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsMarshalByRef");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsPrimitive() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsPrimitive");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsValueType() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsValueType");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetIsSignatureType() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("get_IsSignatureType");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetField(const util::String& name,
                          const BindingFlags flags = static_cast<BindingFlags>(static_cast<int>(BindingFlags::Instance) | static_cast<int>(BindingFlags::Static) | static_cast<int>(
                              BindingFlags::Public))) -> FieldInfo* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Type", "System", "MemberInfo")->Get<Method>("GetField", {"System.String name", "System.Reflection.BindingFlags"});
                }
                if (method) {
                    return method->Invoke<FieldInfo*>(this, String::New(name), flags);
                }
                return nullptr;
            }
        };

        struct String : Object {
            int32_t m_stringLength{0};
            wchar_t m_firstChar[32]{};

            [[nodiscard]] auto ToString() const -> util::String {
                return Encode::wchar_to_char(m_firstChar, m_stringLength);
            }

            auto operator[](const int i) const -> wchar_t {
                return m_firstChar[i];
            }

            auto operator=(const util::String& newString) const -> String* {
                return New(newString);
            }

            auto operator==(const std::wstring& newString) const -> bool {
                return Equals(newString);
            }

            auto Clear() -> void {
                memset(m_firstChar, 0, m_stringLength);
                m_stringLength = 0;
            }

            [[nodiscard]] auto Equals(const std::wstring& newString) const -> bool {
                if (newString.size() != m_stringLength) {
                    return false;
                }
                if (std::memcmp(newString.data(), m_firstChar, m_stringLength) != 0) {
                    return false;
                }
                return true;
            }

            static auto New(const util::String& str) -> String* {
                if (mode_ == Mode::Il2Cpp) {
                    return UnityResolve::Invoke<String*, const char*>("il2cpp_string_new", str.data());
                }
                return UnityResolve::Invoke<String*, void*, const char*>("mono_string_new", UnityResolve::Invoke<void*>("mono_get_root_domain"), str.data());
            }
        };

        template<typename T>
        struct Array : Object {
            struct {
                std::uintptr_t length;
                std::int32_t lower_bound;
            }* bounds{nullptr};

            std::uintptr_t max_length{0};
            T** vector{};

            auto GetData() -> uintptr_t {
                return reinterpret_cast<uintptr_t>(&vector);
            }

            auto operator[](const unsigned int m_uIndex) -> T& {
                return *reinterpret_cast<T*>(GetData() + sizeof(T) * m_uIndex);
            }

            auto At(const unsigned int m_uIndex) -> T& {
                return operator[](m_uIndex);
            }

            auto Insert(T* m_pArray, uintptr_t m_uSize, const uintptr_t m_uIndex = 0) -> void {
                if ((m_uSize + m_uIndex) >= max_length) {
                    if (m_uIndex >= max_length) {
                        return;
                    }

                    m_uSize = max_length - m_uIndex;
                }

                for (uintptr_t u = 0; m_uSize > u; ++u) {
                    operator[](u + m_uIndex) = m_pArray[u];
                }
            }

            auto Fill(T m_tValue) -> void {
                for (uintptr_t u = 0; max_length > u; ++u) {
                    operator[](u) = m_tValue;
                }
            }

            auto RemoveAt(const unsigned int m_uIndex) -> void {
                if (m_uIndex >= max_length) {
                    return;
                }

                if (max_length > (m_uIndex + 1)) {
                    for (auto u = m_uIndex; (max_length - m_uIndex) > u; ++u) {
                        operator[](u) = operator[](u + 1);
                    }
                }

                --max_length;
            }

            auto RemoveRange(const unsigned int m_uIndex, unsigned int m_uCount) -> void {
                if (m_uCount == 0) {
                    m_uCount = 1;
                }

                const auto m_uTotal = m_uIndex + m_uCount;
                if (m_uTotal >= max_length) {
                    return;
                }

                if (max_length > (m_uTotal + 1)) {
                    for (auto u = m_uIndex; (max_length - m_uTotal) >= u; ++u) {
                        operator[](u) = operator[](u + m_uCount);
                    }
                }

                max_length -= m_uCount;
            }

            auto RemoveAll() -> void {
                if (max_length > 0) {
                    memset(GetData(), 0, sizeof(Type) * max_length);
                    max_length = 0;
                }
            }

            auto ToVector() -> std::vector<T, mi_stl_allocator<T>> {
                std::vector<T, mi_stl_allocator<T>> rs(this->max_length);
                memcpy_fast(rs.data(), &vector, sizeof(T) * this->max_length);
                return rs;
            }


            auto Resize(int newSize) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("Array")->Get<Method>("Resize");
                }
                if (method) {
                    return method->Invoke<void>(this, newSize);
                }
            }

            static auto New(const Class* kalss, const std::uintptr_t size) -> Array* {
                if (mode_ == Mode::Il2Cpp) {
                    return UnityResolve::Invoke<Array*, void*, std::uintptr_t>("il2cpp_array_new", kalss->address, size);
                }
                return UnityResolve::Invoke<Array*, void*, void*, std::uintptr_t>("mono_array_new", pDomain, kalss->address, size);
            }
        };

        template<typename Type>
        struct List : Object {
            Array<Type>* pList;
            int size{};
            int version{};
            void* syncRoot{};

            auto ToArray() -> Array<Type>* {
                return pList;
            }

            static auto New(const Class* kalss, const std::uintptr_t size) -> List* {
                auto pList = new List<Type>();
                pList->pList = Array<Type>::New(kalss, size);
                pList->size = size;
                return pList;
            }

            auto operator[](const unsigned int m_uIndex) -> Type& {
                return pList->At(m_uIndex);
            }

            auto Add(Type pDate) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("Add");
                }
                if (method) {
                    return method->Invoke<void>(this, pDate);
                }
            }

            auto Remove(Type pDate) -> bool {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("Remove");
                }
                if (method) {
                    return method->Invoke<bool>(this, pDate);
                }
                return false;
            }

            auto RemoveAt(int index) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("RemoveAt");
                }
                if (method) {
                    return method->Invoke<void>(this, index);
                }
            }

            auto ForEach(void (*action)(Type pDate)) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("ForEach");
                }
                if (method) {
                    return method->Invoke<void>(this, action);
                }
            }

            auto GetRange(int index, int count) -> List* {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("GetRange");
                }
                if (method) {
                    return method->Invoke<List*>(this, index, count);
                }
                return nullptr;
            }

            auto Clear() -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("Clear");
                }
                if (method) {
                    return method->Invoke<void>(this);
                }
            }

            auto Sort(int (*comparison)(Type* pX, Type* pY)) -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("List`1")->Get<Method>("Sort", {"*"});
                }
                if (method) {
                    return method->Invoke<void>(this, comparison);
                }
            }
        };

        template<typename TKey, typename TValue>
        struct Dictionary : Object {
            struct Entry {
                int iHashCode;
                int iNext;
                TKey tKey;
                TValue tValue;
            };

            Array<int>* pBuckets;
            Array<Entry*>* pEntries;
            int iCount;
            int iVersion;
            int iFreeList;
            int iFreeCount;
            void* pComparer;
            void* pKeys;
            void* pValues;

            auto GetEntry() -> Entry* {
                return reinterpret_cast<Entry*>(pEntries->GetData());
            }

            auto GetKeyByIndex(const int iIndex) -> TKey& {
                Entry* pEntry = GetEntry();
                if (pEntry) {
                    return pEntry[iIndex].tKey;
                }

                TKey v;
                return v;
            }

            auto GetValueByIndex(const int iIndex) -> TValue& {
                Entry* pEntry = GetEntry();
                if (pEntry) {
                    return pEntry[iIndex].tValue;
                }

                TValue v;
                return v;
            }

            auto GetValueByKey(const TKey tKey) -> TValue {
                TValue tValue = {0};
                for (auto i = 0; i < iCount; i++) {
                    if (GetEntry()[i].tKey == tKey) {
                        tValue = GetEntry()[i].tValue;
                    }
                }
                return tValue;
            }

            auto operator[](const TKey tKey) const -> TValue {
                return GetValueByKey(tKey);
            }
        };

        struct UnityObject : Object {
            void* m_CachedPtr;

            auto GetName() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("get_name");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            auto ToString() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("ToString");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            static auto ToString(UnityObject* obj) -> String* {
                if (!obj) {
                    return {};
                }
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("ToString", {"*"});
                }
                if (method) {
                    return method->Invoke<String*>(obj);
                }
                return {};
            }

            static auto Instantiate(UnityObject* original) -> UnityObject* {
                if (!original) {
                    return nullptr;
                }
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("Instantiate", {"*"});
                }
                if (method) {
                    return method->Invoke<UnityObject*>(original);
                }
                return nullptr;
            }

            static auto Destroy(UnityObject* original) -> void {
                if (!original) {
                    return;
                }
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("Destroy", {"*"});
                }
                if (method) {
                    return method->Invoke<void>(original);
                }
            }

            template<typename T>
            static auto find_object_from_instance_id(const int32_t _instance_id) -> T {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<Method>("FindObjectFromInstanceID");
                return method->Invoke<T>(_instance_id);
            }
        };

        struct Component : UnityObject {
            auto GetTransform() -> Transform* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("get_transform");
                }
                if (method) {
                    return method->Invoke<Transform*>(this);
                }
                return nullptr;
            }

            auto get_game_object() -> GameObject* {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("get_gameObject");
                return method->Invoke<GameObject*>(this);
            }

            auto GetTag() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("get_tag");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            template<typename T>
            auto GetComponentsInChildren() -> std::vector<T> {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentsInChildren");
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponentsInChildren(Class* pClass) -> std::vector<T> {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentsInChildren", {"System.Type"});
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this, pClass->GetType())->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponents() -> std::vector<T> {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponents");
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponents(Class* pClass) -> std::vector<T> {
                static Method* method;
                static void* obj;

                if (!method || !obj) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponents", {"System.Type"});
                    obj = pClass->GetType();
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this, obj)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponentsInParent() -> std::vector<T> {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentsInParent");
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponentsInParent(Class* pClass) -> std::vector<T> {
                static Method* method;
                static void* obj;

                if (!method || !obj) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentsInParent", {"System.Type"});
                    obj = pClass->GetType();
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this, obj)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponentInChildren(Class* pClass) -> T {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentInChildren", {"System.Type"});
                }
                if (method) {
                    return method->Invoke<T>(this, pClass->GetType());
                }
                return T();
            }

            template<typename T>
            auto get_component_in_parent(Class* pClass) -> T {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Component")->Get<Method>("GetComponentInParent", {"System.Type"});
                }
                if (method) {
                    return method->Invoke<T>(this, pClass->GetType());
                }
                return T();
            }
        };

        struct Camera : Component {
            enum class Eye : int { Left, Right, Mono };

            static auto GetMain() -> Camera* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("get_main");
                }
                if (method) {
                    return method->Invoke<Camera*>();
                }
                return nullptr;
            }

            static auto GetCurrent() -> Camera* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("get_current");
                }
                if (method) {
                    return method->Invoke<Camera*>();
                }
                return nullptr;
            }

            static auto GetAllCount() -> int {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("get_allCamerasCount");
                }
                if (method) {
                    return method->Invoke<int>();
                }
                return 0;
            }

            static auto GetAllCamera() -> std::vector<Camera*, mi_stl_allocator<Camera*>> {
                static Method* method;
                static Class* klass;

                if (!method || !klass) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("GetAllCameras", {"*"});
                    klass = Get("UnityEngine.CoreModule.dll")->Get("Camera");
                }

                if (method && klass) {
                    if (const int count = GetAllCount(); count != 0) {
                        const auto array = Array<Camera*>::New(klass, count);
                        method->Invoke<int>(array);
                        return array->ToVector();
                    }
                }

                return {};
            }

            auto GetDepth() -> float {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("get_depth");
                }
                if (method) {
                    return method->Invoke<float>(this);
                }
                return 0.0f;
            }

            auto SetDepth(const float depth) -> void {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("set_depth", {"*"});
                }
                if (method) {
                    return method->Invoke<void>(this, depth);
                }
            }

            auto SetFoV(const float fov) -> void {
                static Method* method_fieldOfView;
                if (!method_fieldOfView) {
                    method_fieldOfView = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("set_fieldOfView", {"*"});
                }
                if (method_fieldOfView) {
                    return method_fieldOfView->Invoke<void>(this, fov);
                }
            }

            auto GetFoV() -> float {
                static Method* method_fieldOfView;
                if (!method_fieldOfView) {
                    method_fieldOfView = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("get_fieldOfView");
                }
                if (method_fieldOfView) {
                    return method_fieldOfView->Invoke<float>(this);
                }
                return 0.0f;
            }

            auto WorldToScreenPoint(const Vector3& position, const Eye eye = Eye::Mono) -> Vector3 {
                static Method* method;
                if (!method) {
                    if (mode_ == Mode::Mono) {
                        method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("WorldToScreenPoint_Injected");
                    } else {
                        method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>("WorldToScreenPoint", {"*", "*"});
                    }
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vec3{};
                    method->Invoke<void>(this, position, eye, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Vector3>(this, position, eye);
                }
                return {};
            }

            auto ScreenToWorldPoint(const Vector3& position, const Eye eye = Eye::Mono) -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>(mode_ == Mode::Mono ? "ScreenToWorldPoint_Injected" : "ScreenToWorldPoint");
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vec3{};
                    method->Invoke<void>(this, position, eye, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Vector3>(this, position, eye);
                }
                return {};
            }

            auto camera_to_world_matrix() -> Matrix4x4 {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>(mode_ == Mode::Mono ? "get_cameraToWorldMatrix_Injected" : "get_cameraToWorldMatrix");
                return method->Invoke<Matrix4x4>(this);
            }

            auto get_projection_matrix() -> Matrix4x4 {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>(mode_ == Mode::Mono ? "get_projectionMatrix_Injected" : "get_projectionMatrix");
                return method->Invoke<Matrix4x4>(this);
            }

            auto ScreenPointToRay(const Vector2& position, const Eye eye = Eye::Mono) -> Ray {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Camera")->Get<Method>(mode_ == Mode::Mono ? "ScreenPointToRay_Injected" : "ScreenPointToRay");
                }
                if (mode_ == Mode::Mono && method) {
                    Ray ray{};
                    method->Invoke<void>(this, position, eye, &ray);
                    return ray;
                }
                if (method) {
                    return method->Invoke<Ray>(this, position, eye);
                }
                return {};
            }
        };

        struct Transform : Component {
            auto GetPosition() -> Vector3 {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_position");
                return method->Invoke<Vector3>(this);
            }

            auto SetPosition(const Vector3& position) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("set_position");
                }
                return method->Invoke<void, Transform*, Vector3>(this, position);
            }

            auto GetRight() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_right");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto SetRight(const Vector3& value) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("set_right");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }

            auto GetUp() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_up");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto SetUp(const Vector3& value) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("set_up");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }

            auto GetForward() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_forward");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto SetForward(const Vector3& value) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("set_forward");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }

            auto GetRotation() -> Quaternion {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "get_rotation_Injected" : "get_rotation");
                }
                if (mode_ == Mode::Mono && method) {
                    Quaternion vec3{};
                    method->Invoke<void>(this, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Quaternion>(this);
                }
                return {};
            }

            auto SetRotation(const Quaternion& position) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "set_rotation_Injected" : "set_rotation");
                }
                if (mode_ == Mode::Mono && method) {
                    return method->Invoke<void>(this, &position);
                }
                if (method) {
                    return method->Invoke<void>(this, position);
                }
            }

            auto GetLocalPosition() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "get_localPosition_Injected" : "get_localPosition");
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vec3{};
                    method->Invoke<void>(this, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto SetLocalPosition(const Vector3& position) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "set_localPosition_Injected" : "set_localPosition");
                }
                if (mode_ == Mode::Mono && method) {
                    return method->Invoke<void>(this, &position);
                }
                if (method) {
                    return method->Invoke<void>(this, position);
                }
            }

            auto GetLocalRotation() -> Quaternion {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "get_localRotation_Injected" : "get_localRotation");
                }
                if (mode_ == Mode::Mono && method) {
                    Quaternion vec3{};
                    method->Invoke<void>(this, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Quaternion>(this);
                }
                return {};
            }

            auto SetLocalRotation(const Quaternion& position) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "set_localRotation_Injected" : "set_localRotation");
                }
                if (mode_ == Mode::Mono && method) {
                    return method->Invoke<void>(this, &position);
                }
                if (method) {
                    return method->Invoke<void>(this, position);
                }
            }

            auto GetLocalScale() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_localScale_Injected");
                }
                Vector3 vec3{};
                method->Invoke<void>(this, &vec3);
                return vec3;
            }

            auto SetLocalScale(const Vector3& position) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "set_localScale_Injected" : "set_localScale");
                }
                if (mode_ == Mode::Mono && method) {
                    return method->Invoke<void>(this, &position);
                }
                if (method) {
                    return method->Invoke<void>(this, position);
                }
            }

            auto GetChildCount() -> int {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("get_childCount");
                }
                if (method) {
                    return method->Invoke<int>(this);
                }
                return 0;
            }

            auto GetChild(const int index) -> Transform* {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("GetChild");
                }
                if (method) {
                    return method->Invoke<Transform*>(this, index);
                }
                return nullptr;
            }

            auto GetRoot() -> Transform* {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("GetRoot");
                }
                if (method) {
                    return method->Invoke<Transform*>(this);
                }
                return nullptr;
            }

            auto GetParent() -> Transform* {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("GetParent");
                }
                if (method) {
                    return method->Invoke<Transform*>(this);
                }
                return nullptr;
            }

            auto GetLossyScale() -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "get_lossyScale_Injected" : "get_lossyScale");
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vec3{};
                    method->Invoke<void>(this, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto TransformPoint(const Vector3& position) -> Vector3 {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>(mode_ == Mode::Mono ? "TransformPoint_Injected" : "TransformPoint");
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vec3{};
                    method->Invoke<void>(this, position, &vec3);
                    return vec3;
                }
                if (method) {
                    return method->Invoke<Vector3>(this, position);
                }
                return {};
            }

            auto LookAt(const Vector3& worldPosition) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("LookAt", {"Vector3"});
                }
                if (method) {
                    return method->Invoke<void>(this, worldPosition);
                }
            }

            auto Rotate(const Vector3& eulers) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Transform")->Get<Method>("Rotate", {"Vector3"});
                }
                if (method) {
                    return method->Invoke<void>(this, eulers);
                }
            }
        };

        struct GameObject : UnityObject {
            static auto Create(GameObject* obj, const util::String& name) -> void {
                if (!obj) {
                    return;
                }
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("Internal_CreateGameObject");
                }
                if (method) {
                    method->Invoke<void, GameObject*, String*>(obj, String::New(name));
                }
            }

            static auto FindGameObjectsWithTag(const util::String& name) -> std::vector<GameObject*, mi_stl_allocator<GameObject*>> {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("FindGameObjectsWithTag");
                }
                if (method) {
                    const auto array = method->Invoke<Array<GameObject*>*>(String::New(name));
                    return array->ToVector();
                }
                return {};
            }

            static auto Find(const util::String& name) -> GameObject* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("Find");
                }
                if (method) {
                    return method->Invoke<GameObject*>(String::New(name));
                }
                return nullptr;
            }

            auto GetActive() -> bool {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_active");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto SetActive(const bool value) -> void {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("set_active");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }

            auto GetActiveSelf() -> bool {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_activeSelf");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto get_active_in_hierarchy() -> bool {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_activeInHierarchy");
                return method->Invoke<bool>(this);
            }

            auto get_layer() -> int {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_layer");
                return method->Invoke<int>(this);
            }

            auto set_layer(const int layer) -> void {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("set_layer");
                return method->Invoke<void>(this, layer);
            }

            auto GetIsStatic() -> bool {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_isStatic");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto GetTransform() -> Transform* {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_transform");
                }
                if (method) {
                    return method->Invoke<Transform*>(this);
                }
                return nullptr;
            }

            auto GetTag() -> String* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("get_tag");
                }
                if (method) {
                    return method->Invoke<String*>(this);
                }
                return {};
            }

            template<typename T>
            auto GetComponent() -> T {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("GetComponent");
                }
                if (method) {
                    return method->Invoke<T>(this);
                }
                return T();
            }

            template<typename T>
            auto GetComponent(Class* type) -> T {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("GetComponent", {"System.Type"});
                }
                if (method) {
                    return method->Invoke<T>(this, type->GetType());
                }
                return T();
            }

            template<typename T>
            auto GetComponentInChildren(Class* type) -> T {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("GetComponentInChildren", {"System.Type"});
                }
                if (method) {
                    return method->Invoke<T>(this, type->GetType());
                }
                return T();
            }

            template<typename T>
            auto GetComponentInParent(Class* _type) -> T {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("GetComponentInParent", {"System.Type"});
                return method->Invoke<T>(this, _type->GetType());
            }

            template<typename T>
            auto GetComponents(Class* type,
                               bool useSearchTypeAsArrayReturnType = false,
                               bool recursive = false,
                               bool includeInactive = true,
                               bool reverse = false,
                               List<T>* resultList = nullptr) -> std::vector<T> {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("GameObject")->Get<Method>("GetComponentsInternal");
                }
                if (method) {
                    return method->Invoke<Array<T>*>(this, type->GetType(), useSearchTypeAsArrayReturnType, recursive, includeInactive, reverse, resultList)->ToVector();
                }
                return {};
            }

            template<typename T>
            auto GetComponentsInChildren(Class* type, const bool includeInactive = false) -> std::vector<T> {
                return GetComponents<T>(type, false, true, includeInactive, false, nullptr);
            }

            template<typename T>
            auto GetComponentsInParent(Class* type, const bool includeInactive = false) -> std::vector<T> {
                return GetComponents<T>(type, false, true, includeInactive, true, nullptr);
            }
        };

        struct LayerMask : Object {
            int m_Mask;

            static auto NameToLayer(const util::String& layerName) -> int {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("LayerMask")->Get<Method>("NameToLayer");
                }
                if (method) {
                    return method->Invoke<int>(String::New(layerName));
                }
                return 0;
            }

            static auto LayerToName(const int _layer) -> String* {
                static Method* method = Get("UnityEngine.CoreModule.dll")->Get("LayerMask")->Get<Method>("LayerToName");
                return method->Invoke<String*>(_layer);
            }
        };

        struct Rigidbody : Component {
            auto GetDetectCollisions() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("Rigidbody")->Get<Method>("get_detectCollisions");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto SetDetectCollisions(const bool value) -> void {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("Rigidbody")->Get<Method>("set_detectCollisions");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }

            auto GetVelocity() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("Rigidbody")->Get<Method>(mode_ == Mode::Mono ? "get_velocity_Injected" : "get_velocity");
                }
                if (mode_ == Mode::Mono && method) {
                    Vector3 vector;
                    method->Invoke<void>(this, &vector);
                    return vector;
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto SetVelocity(Vector3 value) -> void {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("Rigidbody")->Get<Method>(mode_ == Mode::Mono ? "set_velocity_Injected" : "set_velocity");
                }
                if (mode_ == Mode::Mono && method) {
                    return method->Invoke<void>(this, &value);
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }
        };

        struct Collider : Component {
            auto GetBounds() -> Bounds {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("Collider")->Get<Method>("get_bounds_Injected");
                }
                if (method) {
                    Bounds bounds;
                    method->Invoke<void>(this, &bounds);
                    return bounds;
                }
                return {};
            }
        };

        struct Mesh : UnityObject {
            auto GetBounds() -> Bounds {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Mesh")->Get<Method>("get_bounds_Injected");
                }
                if (method) {
                    Bounds bounds;
                    method->Invoke<void>(this, &bounds);
                    return bounds;
                }
                return {};
            }
        };

        struct CapsuleCollider : Collider {
            auto GetCenter() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("CapsuleCollider")->Get<Method>("get_center");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto GetDirection() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("CapsuleCollider")->Get<Method>("get_direction");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto GetHeightn() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("CapsuleCollider")->Get<Method>("get_height");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto GetRadius() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("CapsuleCollider")->Get<Method>("get_radius");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }
        };

        struct BoxCollider : Collider {
            auto GetCenter() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("BoxCollider")->Get<Method>("get_center");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }

            auto GetSize() -> Vector3 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.PhysicsModule.dll")->Get("BoxCollider")->Get<Method>("get_size");
                }
                if (method) {
                    return method->Invoke<Vector3>(this);
                }
                return {};
            }
        };

        struct Renderer : Component {
            auto GetBounds() -> Bounds {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Renderer")->Get<Method>("get_bounds_Injected");
                }
                if (method) {
                    Bounds bounds;
                    method->Invoke<void>(this, &bounds);
                    return bounds;
                }
                return {};
            }
        };

        struct Behaviour : Component {
            auto GetEnabled() -> bool {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Behaviour")->Get<Method>("get_enabled");
                }
                if (method) {
                    return method->Invoke<bool>(this);
                }
                return false;
            }

            auto SetEnabled(const bool value) -> void {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Behaviour")->Get<Method>("set_enabled");
                }
                if (method) {
                    return method->Invoke<void>(this, value);
                }
            }
        };

        struct MonoBehaviour : Behaviour {};

        struct Physics : Object {
            static auto raycast_all(const Vector3& _origin, const Vector3& _direction) -> Array<RaycastHit>* {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("Physics")->Get<Method>("RaycastAll", {"UnityEngine.Vector3", "UnityEngine.Vector3"});
                return method->Invoke<Array<RaycastHit>*>(_origin, _direction);
            }

            static auto linecast(const Vector3& _origin, const Vector3& _direction, RaycastHit& _hit) -> bool {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("Physics")->Get<Method>("Linecast", {"UnityEngine.Vector3", "UnityEngine.Vector3", "UnityEngine.RaycastHit&"});
                return method->Invoke<bool, const Vector3, const Vector3, RaycastHit&>(_origin, _direction, _hit);
            }

            static auto ignore_collision(Collider* collider1, Collider* collider2) -> void {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("Physics")->Get<Method>("IgnoreCollision1", {"*", "*"});
                return method->Invoke<void>(collider1, collider2);
            }

            static auto get_gravity() -> glm::vec3 {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("Physics")->Get<Method>("get_gravity");
                return method->Invoke<glm::vec3>();
            }

            static auto set_gravity(const glm::vec3& _gravity) -> void {
                static Method* method = Get("UnityEngine.PhysicsModule.dll")->Get("Physics")->Get<Method>("set_gravity");
                return method->Invoke<void>(_gravity);
            }
        };

        struct Texture : UnityObject {
            auto GetWidth() -> int {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Texture")->Get<Method>("get_width");
                }
                if (method) {
                    return method->Invoke<int>(this);
                }
                return 0;
            }

            auto GetHeight() -> int {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Texture")->Get<Method>("get_height");
                }
                if (method) {
                    return method->Invoke<int>(this);
                }
                return 0;
            }

            auto GetNativeTexturePtr() -> void* {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Texture")->Get<Method>("GetNativeTexturePtr");
                }
                if (method) {
                    return method->Invoke<void*>(this);
                }
                return nullptr;
            }
        };

        struct Animator : Behaviour {
            enum class HumanBodyBones : int {
                Hips,
                LeftUpperLeg,
                RightUpperLeg,
                LeftLowerLeg,
                RightLowerLeg,
                LeftFoot,
                RightFoot,
                Spine,
                Chest,
                UpperChest = 54,
                Neck       = 9,
                Head,
                LeftShoulder,
                RightShoulder,
                LeftUpperArm,
                RightUpperArm,
                LeftLowerArm,
                RightLowerArm,
                LeftHand,
                RightHand,
                LeftToes,
                RightToes,
                LeftEye,
                RightEye,
                Jaw,
                LeftThumbProximal,
                LeftThumbIntermediate,
                LeftThumbDistal,
                LeftIndexProximal,
                LeftIndexIntermediate,
                LeftIndexDistal,
                LeftMiddleProximal,
                LeftMiddleIntermediate,
                LeftMiddleDistal,
                LeftRingProximal,
                LeftRingIntermediate,
                LeftRingDistal,
                LeftLittleProximal,
                LeftLittleIntermediate,
                LeftLittleDistal,
                RightThumbProximal,
                RightThumbIntermediate,
                RightThumbDistal,
                RightIndexProximal,
                RightIndexIntermediate,
                RightIndexDistal,
                RightMiddleProximal,
                RightMiddleIntermediate,
                RightMiddleDistal,
                RightRingProximal,
                RightRingIntermediate,
                RightRingDistal,
                RightLittleProximal,
                RightLittleIntermediate,
                RightLittleDistal,
                LastBone = 55
            };

            auto GetBoneTransform(const HumanBodyBones humanBoneId) -> Transform* {
                static Method* method;

                if (!method) {
                    method = Get("UnityEngine.AnimationModule.dll")->Get("Animator")->Get<Method>("GetBoneTransform");
                }
                if (method) {
                    return method->Invoke<Transform*>(this, humanBoneId);
                }
                return nullptr;
            }
        };

        struct Time {
            static auto GetTime() -> float {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Time")->Get<Method>("get_time");
                }
                if (method) {
                    return method->Invoke<float>();
                }
                return 0.0f;
            }

            static auto GetDeltaTime() -> float {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Time")->Get<Method>("get_deltaTime");
                }
                if (method) {
                    return method->Invoke<float>();
                }
                return 0.0f;
            }

            static auto GetFixedDeltaTime() -> float {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Time")->Get<Method>("get_fixedDeltaTime");
                }
                if (method) {
                    return method->Invoke<float>();
                }
                return 0.0f;
            }

            static auto GetTimeScale() -> float {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Time")->Get<Method>("get_timeScale");
                }
                if (method) {
                    return method->Invoke<float>();
                }
                return 0.0f;
            }

            static auto SetTimeScale(const float value) -> void {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Time")->Get<Method>("set_timeScale");
                }
                if (method) {
                    return method->Invoke<void>(value);
                }
            }
        };

        struct Screen {
            static auto get_width() -> Int32 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Screen")->Get<Method>("get_width");
                }
                if (method) {
                    return method->Invoke<int32_t>();
                }
                return 0;
            }

            static auto get_height() -> Int32 {
                static Method* method;
                if (!method) {
                    method = Get("UnityEngine.CoreModule.dll")->Get("Screen")->Get<Method>("get_height");
                }
                if (method) {
                    return method->Invoke<int32_t>();
                }
                return 0;
            }
        };

        struct GC {
            static auto Collect() -> void {
                static Method* method;
                if (!method) {
                    method = Get("mscorlib.dll")->Get("GC")->Get<Method>("Collect");
                }
                if (method) {
                    return method->Invoke<void>();
                }
                return;
            }

            template<typename T>
            static auto gc_get_handle(const uint32_t _handle) -> T* {
                return UnityResolve::Invoke<T*>("il2cpp_gchandle_get_target", _handle);
            }
        };

        template<typename Return, typename... Args>
        static auto Invoke(void* address, Args... args) -> Return {
#if WINDOWS_MODE
            if (address != nullptr) {
                return reinterpret_cast<Return(*)(Args...)>(address)(args...);
            }
#elif LINUX_MODE || ANDROID_MODE || IOS_MODE || HARMONYOS_MODE
            if (address != nullptr)
                return ((Return(*)(Args...))(address))(args...);
#endif
            return Return();
        }
    };

#if WINDOWS_MODE || LINUX_MODE || IOS_MODE /*__cplusplus >= 202002L*/
    static auto DumpToFile(const std::filesystem::path& path) -> void {
        std::ofstream io(path / "dump.cs", std::fstream::out);
        if (!io) {
            return;
        }

        for (const auto& pAssembly : assembly) {
            for (const auto& pClass : pAssembly->classes) {
                io << std::format("\tnamespace: {}", name_map[pClass->namespaze]);
                io << "\n";
                io << std::format("\tAssembly: {}\n", name_map[pAssembly->name]);
                io << std::format("\tAssemblyFile: {} \n", name_map[pAssembly->file]);
                io << std::format("\tclass {}{} ", name_map[pClass->name], " : " + name_map[pClass->parent]);
                io << "{\n\n";
                for (const auto& pField : pClass->fields) {
                    io << std::format("\t\t{:+#06X} | {}{} {};\n", pField.offset, pField.static_field ? "static " : "", name_map[pField.type->name], name_map[pField.name]);
                }
                io << "\n";
                for (const auto& pMethod : pClass->methods) {
                    io << std::format("\t\t[Flags: {:032b}] [ParamsCount: {:04d}] |RVA: {:+#010X}|\n",
                                      pMethod.flags,
                                      pMethod.args.size(),
                                      reinterpret_cast<std::uint64_t>(pMethod.function) - reinterpret_cast<std::uint64_t>(hmodule_));
                    io << std::format("\t\t{}{} {}(", pMethod.static_function ? "static " : "", name_map[pMethod.return_type->name], name_map[pMethod.name]);
                    util::String params{};
                    for (const auto& pArg : pMethod.args) {
                        params += std::format("{} {}, ", name_map[pArg->pType->name], name_map[pArg->name]);
                    }
                    if (!params.empty()) {
                        params.pop_back();
                        params.pop_back();
                    }
                    io << (params.empty() ? "" : params) << ");\n\n";
                }
                io << "\t}\n\n";
            }
        }

        io << '\n';
        io.close();
    }
#endif

private:
    inline static Mode mode_{};
    inline static void* hmodule_;
    inline static std::unordered_map<util::String, void*> address_{};
    inline static void* pDomain{};
};
#endif // UNITYRESOLVE_HPPs
