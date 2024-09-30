// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
{% from 'dawn/cpp_macros.tmpl' import wgpu_string_members with context %}

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
{% set CAPI = metadata.c_prefix %}

{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}

{% set PREFIX = "" if not c_namespace else c_namespace.SNAKE_CASE() + "_" %}
#ifndef {{PREFIX}}{{API}}_CPP_H_
#define {{PREFIX}}{{API}}_CPP_H_

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "{{c_header}}"
#include "{{api}}/{{api}}_cpp_chained_struct.h"
#include "{{api}}/{{api}}_enum_class_bitmasks.h"  // IWYU pragma: export

namespace {{metadata.namespace}} {

namespace detail {
constexpr size_t ConstexprMax(size_t a, size_t b) {
    return a > b ? a : b;
}

template <typename T>
static T& AsNonConstReference(const T& value) {
    return const_cast<T&>(value);
}
}  // namespace detail

{% set c_prefix = metadata.c_prefix %}
{% for constant in by_category["constant"] %}
    {% set type = as_cppType(constant.type.name) %}
    {% set value = c_prefix + "_" +  constant.name.SNAKE_CASE() %}
    static constexpr {{type}} k{{as_cppType(constant.name)}} = {{ value }};
{% endfor %}

{%- macro render_c_actual_arg(arg) -%}
    {%- if arg.annotation == "value" -%}
        {%- if arg.type.category == "object" -%}
            {{as_varName(arg.name)}}.Get()
        {%- elif arg.type.category == "enum" or arg.type.category == "bitmask" -%}
            static_cast<{{as_cType(arg.type.name)}}>({{as_varName(arg.name)}})
        {%- elif arg.type.category == "structure" -%}
            *reinterpret_cast<{{as_cType(arg.type.name)}} const*>(&{{as_varName(arg.name)}})
        {%- elif arg.type.category in ["function pointer", "native"] -%}
            {{as_varName(arg.name)}}
        {%- else -%}
            UNHANDLED
        {%- endif -%}
    {%- else -%}
        reinterpret_cast<{{decorate("", as_cType(arg.type.name), arg)}}>({{as_varName(arg.name)}})
    {%- endif -%}
{%- endmacro -%}

{%- macro render_cpp_to_c_method_call(type, method) -%}
    {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get()
        {%- for arg in method.arguments -%},{{" "}}{{render_c_actual_arg(arg)}}
        {%- endfor -%}
    )
{%- endmacro -%}

//* Although 'optional bool' is defined as an enum value, in C++, we manually implement it to
//* provide conversion utilities.
{% for type in by_category["enum"] if type.name.get() != "optional bool" %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    enum class {{CppType}} : uint32_t {
        {% for value in type.values %}
            {{as_cppEnum(value.name)}} = {{as_cEnum(type.name, value.name)}},
        {% endfor %}
    };
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

{% for type in by_category["bitmask"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    enum class {{CppType}} : uint64_t {
        {% for value in type.values %}
            {{as_cppEnum(value.name)}} = {{as_cEnum(type.name, value.name)}},
        {% endfor %}
    };
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

// TODO(crbug.com/42241461): Update these to not be using the C callback types, and instead be
// defined using C++ types instead. Note that when we remove these, the C++ callback info types
// should also all be removed as they will no longer be necessary given the C++ templated
// functions calls and setter utilities.
{% for type in by_category["function pointer"] %}
    using {{as_cppType(type.name)}} = {{as_cType(type.name)}};
{% endfor %}

// Special class for booleans in order to allow implicit conversions.
{% set BoolCppType = as_cppType(types["bool"].name) %}
{% set BoolCType = as_cType(types["bool"].name) %}
class {{BoolCppType}} {
  public:
    constexpr {{BoolCppType}}() = default;
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{BoolCppType}}(bool value) : mValue(static_cast<{{BoolCType}}>(value)) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    {{BoolCppType}}({{BoolCType}} value): mValue(value) {}

    constexpr operator bool() const { return static_cast<bool>(mValue); }

  private:
    friend struct std::hash<{{BoolCppType}}>;
    // Default to false.
    {{BoolCType}} mValue = static_cast<{{BoolCType}}>(false);
};

// Special class for optional booleans in order to allow conversions.
{% set OptionalBool = types["optional bool"] %}
{% set OptionalBoolCppType = as_cppType(OptionalBool.name) %}
{% set OptionalBoolCType = as_cType(OptionalBool.name) %}
{% set OptionalBoolUndefined = as_cEnum(OptionalBool.name, find_by_name(OptionalBool.values, "undefined").name) %}
class {{OptionalBoolCppType}} {
  public:
    constexpr {{OptionalBoolCppType}}() = default;
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}(bool value) : mValue(static_cast<{{OptionalBoolCType}}>(value)) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}(std::optional<bool> value) :
        mValue(value ? static_cast<{{OptionalBoolCType}}>(*value) : {{OptionalBoolUndefined}}) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}({{OptionalBoolCType}} value): mValue(value) {}

    // Define the values that are equivalent to the enums.
    {% for value in OptionalBool.values %}
        static const {{OptionalBoolCppType}} {{as_cppEnum(value.name)}};
    {% endfor %}

    // Assignment operators.
    {{OptionalBoolCppType}}& operator=(const bool& value) {
        mValue = static_cast<{{OptionalBoolCType}}>(value);
        return *this;
    }
    {{OptionalBoolCppType}}& operator=(const std::optional<bool>& value) {
        mValue = value ? static_cast<{{OptionalBoolCType}}>(*value) : {{OptionalBoolUndefined}};
        return *this;
    }
    {{OptionalBoolCppType}}& operator=(const {{OptionalBoolCType}}& value) {
        mValue = value;
        return *this;
    }

    // Conversion functions.
    operator {{OptionalBoolCType}}() const { return mValue; }
    operator std::optional<bool>() const {
        if (mValue == {{OptionalBoolUndefined}}) {
            return std::nullopt;
        }
        return static_cast<bool>(mValue);
    }

    // Comparison functions.
    bool operator==({{OptionalBoolCType}} rhs) const {
        return mValue == rhs;
    }
    bool operator!=({{OptionalBoolCType}} rhs) const {
        return mValue != rhs;
    }

  private:
    friend struct std::hash<{{OptionalBoolCppType}}>;
    // Default to undefined.
    {{OptionalBoolCType}} mValue = {{OptionalBoolUndefined}};
};
{% for value in OptionalBool.values %}
    inline const {{OptionalBoolCppType}} {{OptionalBoolCppType}}::{{as_cppEnum(value.name)}} = {{OptionalBoolCppType}}({{as_cEnum(OptionalBool.name, value.name)}});
{% endfor %}

// Helper class to wrap Status which allows implicit conversion to bool.
// Used while callers switch to checking the Status enum instead of booleans.
// TODO(crbug.com/42241199): Remove when all callers check the enum.
struct ConvertibleStatus {
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr ConvertibleStatus(Status status) : status(status) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit conversion
    constexpr operator bool() const {
        return status == Status::Success;
    }
    // NOLINTNEXTLINE(runtime/explicit) allow implicit conversion
    constexpr operator Status() const {
        return status;
    }
    Status status;
};

template<typename Derived, typename CType>
class ObjectBase {
  public:
    ObjectBase() = default;
    ObjectBase(CType handle): mHandle(handle) {
        if (mHandle) Derived::{{c_prefix}}AddRef(mHandle);
    }
    ~ObjectBase() {
        if (mHandle) Derived::{{c_prefix}}Release(mHandle);
    }

    ObjectBase(ObjectBase const& other)
        : ObjectBase(other.Get()) {
    }
    Derived& operator=(ObjectBase const& other) {
        if (&other != this) {
            if (mHandle) Derived::{{c_prefix}}Release(mHandle);
            mHandle = other.mHandle;
            if (mHandle) Derived::{{c_prefix}}AddRef(mHandle);
        }

        return static_cast<Derived&>(*this);
    }

    ObjectBase(ObjectBase&& other) {
        mHandle = other.mHandle;
        other.mHandle = 0;
    }
    Derived& operator=(ObjectBase&& other) {
        if (&other != this) {
            if (mHandle) Derived::{{c_prefix}}Release(mHandle);
            mHandle = other.mHandle;
            other.mHandle = 0;
        }

        return static_cast<Derived&>(*this);
    }

    ObjectBase(std::nullptr_t) {}
    Derived& operator=(std::nullptr_t) {
        if (mHandle != nullptr) {
            Derived::{{c_prefix}}Release(mHandle);
            mHandle = nullptr;
        }
        return static_cast<Derived&>(*this);
    }

    bool operator==(std::nullptr_t) const {
        return mHandle == nullptr;
    }
    bool operator!=(std::nullptr_t) const {
        return mHandle != nullptr;
    }

    explicit operator bool() const {
        return mHandle != nullptr;
    }
    CType Get() const {
        return mHandle;
    }
    CType MoveToCHandle() {
        CType result = mHandle;
        mHandle = 0;
        return result;
    }
    static Derived Acquire(CType handle) {
        Derived result;
        result.mHandle = handle;
        return result;
    }

  protected:
    CType mHandle = nullptr;
};

{% macro render_cpp_default_value(member, is_struct, force_default=False) -%}
    {%- if member.json_data.get("no_default", false) -%}
    {%- elif member.annotation in ["*", "const*"] and member.optional or member.default_value == "nullptr" -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "object" and member.optional and is_struct -%}
        {{" "}}= nullptr
    {%- elif member.type.category in ["enum", "bitmask"] and member.default_value != None -%}
        {{" "}}= {{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
    {%- elif member.type.category == "native" and member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- elif member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- elif member.type.category == "structure" and member.annotation == "value" and is_struct -%}
        {{" "}}= {}
    {%- else -%}
        {{assert(member.default_value == None)}}
        {%- if force_default -%}
            {{" "}}= {}
        {%- endif -%}
    {%- endif -%}
{%- endmacro %}

//* This rendering macro should ONLY be used for callback info type functions.
{% macro render_cpp_callback_info_template_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set OriginalMethodName = method.name.CamelCase() %}
    {% set MethodName = OriginalMethodName[:-1] if method.name.chunks[-1] == "2" else OriginalMethodName %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    //* Stripping the 2 at the end of the callback functions for now until we can deprecate old ones.
    //* TODO: crbug.com/dawn/2509 - Remove name handling once old APIs are deprecated.
    {% set CallbackInfoType = (method.arguments|last).type %}
    {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
    {% set SfinaeArg = " = std::enable_if_t<std::is_convertible_v<F, Cb*>>" if not dfn else "" %}
    template <typename F, typename T,
              typename Cb
                {%- if not dfn -%}
                    {{" "}}= void (
                        {%- for arg in CallbackType.arguments -%}
                            {{as_annotated_cppType(arg)}}{{", "}}
                        {%- endfor -%}
                    T userdata)
                {%- endif -%},
              typename{{SfinaeArg}}>
    {{as_cppType(method.return_type.name)}} {{MethodName}}(
        {%- for arg in method.arguments if arg.type.category != "callback info" -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}{{ ", "}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}{{ ", "}}
            {%- endif -%}
        {%- endfor -%}
    {{as_cppType(types["callback mode"].name)}} callbackMode, F callback, T userdata) const
{%- endmacro %}

//* This rendering macro should ONLY be used for callback info type functions.
{% macro render_cpp_callback_info_lambda_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set OriginalMethodName = method.name.CamelCase() %}
    {% set MethodName = OriginalMethodName[:-1] if method.name.chunks[-1] == "2" else OriginalMethodName %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    //* Stripping the 2 at the end of the callback functions for now until we can deprecate old ones.
    //* TODO: crbug.com/dawn/2509 - Remove name handling once old APIs are deprecated.
    {% set CallbackInfoType = (method.arguments|last).type %}
    {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
    {% set SfinaeArg = " = std::enable_if_t<std::is_convertible_v<L, Cb>>" if not dfn else "" %}
    template <typename L,
              typename Cb
                {%- if not dfn -%}
                    {{" "}}= std::function<void(
                        {%- for arg in CallbackType.arguments -%}
                            {%- if not loop.first %}, {% endif -%}
                            {{as_annotated_cppType(arg)}}
                        {%- endfor -%}
                    )>
                {%- endif -%},
              typename{{SfinaeArg}}>
    {{as_cppType(method.return_type.name)}} {{MethodName}}(
        {%- for arg in method.arguments if arg.type.category != "callback info" -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}{{ ", "}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}{{ ", "}}
            {%- endif -%}
        {%- endfor -%}
    {{as_cppType(types["callback mode"].name)}} callbackMode, L callback) const
{%- endmacro %}

//* This rendering macro should NOT be used for callback info type functions.
{% macro render_cpp_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set OriginalMethodName = method.name.CamelCase() %}
    {% set MethodName = OriginalMethodName[:-1] if method.name.chunks[-1] == "f" or method.name.chunks[-1] == "2" else OriginalMethodName %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {{"ConvertibleStatus" if method.return_type.name.get() == "status" else as_cppType(method.return_type.name)}} {{MethodName}}(
        {%- for arg in method.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}
            {%- endif -%}
            {% if not dfn %}{{render_cpp_default_value(arg, False)}}{% endif %}
        {%- endfor -%}
    ) const
{%- endmacro %}

{%- macro render_function_call(function) -%}
    {{as_cMethodNamespaced(None, function.name, c_namespace)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{render_c_actual_arg(arg)}}
        {%- endfor -%}
    )
{%- endmacro -%}

{%- if metadata.namespace != 'wgpu' %}
    // The operators of webgpu_enum_class_bitmasks.h are in the wgpu:: namespace,
    // and need to be imported into this namespace for Argument Dependent Lookup.
    WGPU_IMPORT_BITMASK_OPERATORS
{% endif %}

{% if c_namespace %}
    namespace {{c_namespace.namespace_case()}} {
{% endif %}

{% for type in by_category["object"] %}
    class {{as_cppType(type.name)}};
{% endfor %}

{% for type in by_category["structure"] %}
    struct {{as_cppType(type.name)}};
{% endfor %}

{% macro render_cpp_callback_info_template_method_impl(type, method) %}
    {{render_cpp_callback_info_template_method_declaration(type, method, dfn=True)}} {
        {% set CallbackInfoType = (method.arguments|last).type %}
        {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
        {{as_cType(CallbackInfoType.name)}} callbackInfo = {};
        callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
        callbackInfo.callback = [](
            {%- for arg in CallbackType.arguments -%}
                {{as_annotated_cType(arg)}}{{", "}}
            {%- endfor -%}
        void* callback, void* userdata) {
            auto cb = reinterpret_cast<Cb*>(callback);
            (*cb)(
                {%- for arg in CallbackType.arguments -%}
                    {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}{{", "}}
                {%- endfor -%}
            static_cast<T>(userdata));
        };
        callbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
        callbackInfo.userdata2 = reinterpret_cast<void*>(userdata);
        auto result = {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
            {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                {{render_c_actual_arg(arg)}}{{", "}}
            {%- endfor -%}
        callbackInfo);
        return {{convert_cType_to_cppType(method.return_type, 'value', 'result') | indent(4)}};
    }
{%- endmacro %}

{% macro render_cpp_callback_info_lambda_method_impl(type, method) %}
    {{render_cpp_callback_info_lambda_method_declaration(type, method, dfn=True)}} {
        {% set CallbackInfoType = (method.arguments|last).type %}
        {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
        using F = void (
            {%- for arg in CallbackType.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{as_annotated_cppType(arg)}}
            {%- endfor -%}
        );

        {{as_cType(CallbackInfoType.name)}} callbackInfo = {};
        callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
        if constexpr (std::is_convertible_v<L, F*>) {
            callbackInfo.callback = [](
            {%- for arg in CallbackType.arguments -%}
                {{as_annotated_cType(arg)}}{{", "}}
            {%- endfor -%}
            void* callback, void*) {
                auto cb = reinterpret_cast<F*>(callback);
                (*cb)(
                    {%- for arg in CallbackType.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}
                    {%- endfor -%});
            };
            callbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
            callbackInfo.userdata2 = nullptr;
            auto result = {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
            {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                {{render_c_actual_arg(arg)}}{{", "}}
            {%- endfor -%}
            callbackInfo);
            return {{convert_cType_to_cppType(method.return_type, 'value', 'result') | indent(8)}};
        } else {
            auto* lambda = new L(std::move(callback));
            callbackInfo.callback = [](
                {%- for arg in CallbackType.arguments -%}
                    {{as_annotated_cType(arg)}}{{", "}}
                {%- endfor -%}
            void* callback, void*) {
                std::unique_ptr<L> lambda(reinterpret_cast<L*>(callback));
                (*lambda)(
                    {%- for arg in CallbackType.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}
                    {%- endfor -%});
            };
            callbackInfo.userdata1 = reinterpret_cast<void*>(lambda);
            callbackInfo.userdata2 = nullptr;
            auto result = {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
            {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                {{render_c_actual_arg(arg)}}{{", "}}
            {%- endfor -%}
            callbackInfo);
            return {{convert_cType_to_cppType(method.return_type, 'value', 'result') | indent(8)}};
        }
    }
{%- endmacro %}

{% macro render_cpp_method_impl(type, method) %}
    {{render_cpp_method_declaration(type, method, dfn=True)}} {
        {% for arg in method.arguments if arg.type.has_free_members_function and arg.annotation == '*' %}
            *{{as_varName(arg.name)}} = {{as_cppType(arg.type.name)}}();
        {% endfor %}
        {% if method.return_type.name.concatcase() == "void" %}
            {{render_cpp_to_c_method_call(type, method)}};
        {% else %}
            auto result = {{render_cpp_to_c_method_call(type, method)}};
            return {{convert_cType_to_cppType(method.return_type, 'value', 'result') | indent(8)}};
        {% endif %}
    }
{%- endmacro %}

{% for type in by_category["object"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    class {{CppType}} : public ObjectBase<{{CppType}}, {{CType}}> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        {% for method in type.methods %}
            {% if has_callbackInfoStruct(method) %}
                {{render_cpp_callback_info_template_method_declaration(type, method)|indent}};
                {{render_cpp_callback_info_lambda_method_declaration(type, method)|indent}};
            {% else %}
                inline {{render_cpp_method_declaration(type, method)}};
            {% endif %}
        {% endfor %}

        {% if CppType == "Instance" %}
            inline WaitStatus WaitAny(Future f, uint64_t timeout) const;
        {% endif %}

      private:
        friend ObjectBase<{{CppType}}, {{CType}}>;
        static inline void {{c_prefix}}AddRef({{CType}} handle);
        static inline void {{c_prefix}}Release({{CType}} handle);
    };

{% endfor %}

// ChainedStruct
{% set c_prefix = metadata.c_prefix %}
static_assert(sizeof(ChainedStruct) == sizeof({{c_prefix}}ChainedStruct),
    "sizeof mismatch for ChainedStruct");
static_assert(alignof(ChainedStruct) == alignof({{c_prefix}}ChainedStruct),
    "alignof mismatch for ChainedStruct");
static_assert(offsetof(ChainedStruct, nextInChain) == offsetof({{c_prefix}}ChainedStruct, next),
    "offsetof mismatch for ChainedStruct::nextInChain");
static_assert(offsetof(ChainedStruct, sType) == offsetof({{c_prefix}}ChainedStruct, sType),
    "offsetof mismatch for ChainedStruct::sType");

//* Special structures that require some custom code generation.
{% set SpecialStructures = ["device descriptor"] %}

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set Out = "Out" if type.output else "" %}
    {% set const = "const" if not type.output else "" %}
    {% if type.chained %}
        {% for root in type.chain_roots %}
            // Can be chained in {{as_cppType(root.name)}}
        {% endfor %}
        struct {{as_cppType(type.name)}} : ChainedStruct{{Out}} {
            inline {{as_cppType(type.name)}}();

            struct Init;
            inline {{as_cppType(type.name)}}(Init&& init);
    {% else %}
        struct {{as_cppType(type.name)}} {
            {% if type.has_free_members_function %}
                inline {{as_cppType(type.name)}}();
            {% endif %}
    {% endif %}
        {% if type.has_free_members_function %}
            inline ~{{as_cppType(type.name)}}();
            {{as_cppType(type.name)}}(const {{as_cppType(type.name)}}&) = delete;
            {{as_cppType(type.name)}}& operator=(const {{as_cppType(type.name)}}&) = delete;
            inline {{as_cppType(type.name)}}({{as_cppType(type.name)}}&&);
            inline {{as_cppType(type.name)}}& operator=({{as_cppType(type.name)}}&&);
        {% endif %}
        inline operator const {{as_cType(type.name)}}&() const noexcept;

        {% if type.extensible %}
            ChainedStruct{{Out}} {{const}} * nextInChain = nullptr;
        {% endif %}
        {% for member in type.members %}
            {% set member_declaration = as_annotated_cppType(member, type.has_free_members_function) + render_cpp_default_value(member, True, type.has_free_members_function) %}
            {% if type.chained and loop.first %}
                //* Align the first member after ChainedStruct to match the C struct layout.
                //* It has to be aligned both to its natural and ChainedStruct's alignment.
                static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct{{out}}), alignof({{decorate("", as_cppType(member.type.name), member)}}));
                alignas(kFirstMemberAlignment) {{member_declaration}};
            {% else %}
                {{member_declaration}};
            {% endif %}
        {% endfor %}

        //* Custom string constructors
        {% if type.name.get() == "string view" %}
            {{wgpu_string_members(as_cppType(type.name)) | indent(4)}}
        {% endif %}

        {% if type.has_free_members_function %}

          private:
        {% if type.has_free_members_function %}
                inline void FreeMembers();
        {% endif %}
            static inline void Reset({{as_cppType(type.name)}}& value);
        {% endif %}
    };

{% endfor %}

//* Device descriptor is specially implemented in C++ in order to hide callback info. Note that
//* this is placed at the end of the structs and works for the device descriptor because no other
//* structs include it as a member. In the future for these special structs, we may need to add
//* a way to order the definitions w.r.t the topology of the structs.
{% set type = types["device descriptor"] %}
{% set CppType = as_cppType(type.name) %}
namespace detail {
struct {{CppType}} {
    ChainedStruct const * nextInChain = nullptr;
    {% for member in type.members %}
        {% if member.type.category != "callback info" %}
            {{as_annotated_cppType(member, type.has_free_members_function) + render_cpp_default_value(member, True, type.has_free_members_function)}};
        {% else %}
            {{as_annotated_cType(member)}} = {{CAPI}}_{{member.name.SNAKE_CASE()}}_INIT;
        {% endif %}
    {% endfor %}
};
}  // namespace detail
struct {{CppType}} : protected detail::{{CppType}} {
    inline operator const {{as_cType(type.name)}}&() const noexcept;

    using detail::{{CppType}}::nextInChain;
    {% for member in type.members %}
        {% if member.type.category != "callback info" %}
            using detail::{{CppType}}::{{as_varName(member.name)}};
        {% endif %}
    {% endfor %}

    inline {{CppType}}();
    struct Init;
    inline {{CppType}}(Init&& init);

    template <typename F, typename T,
              typename Cb = void (const Device& device, DeviceLostReason reason, const char * message, T userdata),
              typename = std::enable_if_t<std::is_convertible_v<F, Cb*>>>
    void SetDeviceLostCallback(CallbackMode callbackMode, F callback, T userdata);
    template <typename L,
              typename Cb = std::function<void(const Device& device, DeviceLostReason reason, const char * message)>,
              typename = std::enable_if_t<std::is_convertible_v<L, Cb>>>
    void SetDeviceLostCallback(CallbackMode callbackMode, L callback);

    template <typename F, typename T,
              typename Cb = void (const Device& device, ErrorType type, const char * message, T userdata),
              typename = std::enable_if_t<std::is_convertible_v<F, Cb*>>>
    void SetUncapturedErrorCallback(F callback, T userdata);
    template <typename L,
              typename Cb = std::function<void(const Device& device, ErrorType type, const char * message)>,
              typename = std::enable_if_t<std::is_convertible_v<L, Cb>>>
    void SetUncapturedErrorCallback(L callback);
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
// error: 'offsetof' within non-standard-layout type '{{metadata.namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    // {{CppType}} implementation
    {% if type.chained %}
        {% set Out = "Out" if type.output else "" %}
        {% set const = "const" if not type.output else "" %}
        {{CppType}}::{{CppType}}()
          : ChainedStruct{{Out}} { nullptr, SType::{{type.name.CamelCase()}} } {}
        struct {{CppType}}::Init {
            ChainedStruct{{Out}} * {{const}} nextInChain;
            {% for member in type.members %}
                {% set member_declaration = as_annotated_cppType(member, type.has_free_members_function) + render_cpp_default_value(member, True, type.has_free_members_function) %}
                {{member_declaration}};
            {% endfor %}
        };
        {{CppType}}::{{CppType}}({{CppType}}::Init&& init)
          : ChainedStruct{{Out}} { init.nextInChain, SType::{{type.name.CamelCase()}} }
            {%- for member in type.members -%},{{" "}}
                {{as_varName(member.name)}}(std::move(init.{{as_varName(member.name)}}))
            {%- endfor -%}
            {}
    {% elif type.has_free_members_function %}
        {{CppType}}::{{CppType}}() = default;
    {% endif %}
    {% if type.has_free_members_function %}
        {{CppType}}::~{{CppType}}() {
            FreeMembers();
        }

        {{CppType}}::{{CppType}}({{CppType}}&& rhs)
            : {% for member in type.members %}
            {%- set memberName = member.name.camelCase() -%}
            {{memberName}}(rhs.{{memberName}}){% if not loop.last %},{{"\n            "}}{% endif %}
        {% endfor -%}
        {
            Reset(rhs);
        }

        {{CppType}}& {{CppType}}::operator=({{CppType}}&& rhs) {
            if (&rhs == this) {
                return *this;
            }
            FreeMembers();
            {% for member in type.members %}
                ::{{metadata.namespace}}::detail::AsNonConstReference(this->{{member.name.camelCase()}}) = std::move(rhs.{{member.name.camelCase()}});
            {% endfor %}
            Reset(rhs);
            return *this;
        }

        {% if type.has_free_members_function %}
            void {{CppType}}::FreeMembers() {
                if (
                    {%- for member in type.members if member.annotation != 'value' %}
                        {% if not loop.first %} || {% endif -%}
                        this->{{member.name.camelCase()}} != nullptr
                    {%- endfor -%}
                ) {
                    {{as_cMethodNamespaced(type.name, Name("free members"), c_namespace)}}(
                        *reinterpret_cast<{{CType}}*>(this));
                }
            }
        {% endif %}

        // static
        void {{CppType}}::Reset({{CppType}}& value) {
            {{CppType}} defaultValue{};
            {% for member in type.members %}
                ::{{metadata.namespace}}::detail::AsNonConstReference(value.{{member.name.camelCase()}}) = defaultValue.{{member.name.camelCase()}};
            {% endfor %}
        }
    {% endif %}

    {{CppType}}::operator const {{CType}}&() const noexcept {
        return *reinterpret_cast<const {{CType}}*>(this);
    }

    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");
    {% if type.extensible %}
        static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                "offsetof mismatch for {{CppType}}::nextInChain");
    {% endif %}
    {% for member in type.members %}
        {% set memberName = member.name.camelCase() %}
        static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                "offsetof mismatch for {{CppType}}::{{memberName}}");
    {% endfor %}

{% endfor %}
//* Special implementation for device descriptor.
{% set type = types["device descriptor"] %}
{% set CppType = as_cppType(type.name) %}
{% set CType = as_cType(type.name) %}
// {{CppType}} implementation

{{CppType}}::operator const {{CType}}&() const noexcept {
    return *reinterpret_cast<const {{CType}}*>(this);
}

{{CppType}}::{{CppType}}() : detail::{{CppType}} {} {
    static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                "offsetof mismatch for {{CppType}}::nextInChain");
    {% for member in type.members %}
        {% set memberName = member.name.camelCase() %}
        static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                "offsetof mismatch for {{CppType}}::{{memberName}}");
    {% endfor %}
}

struct {{CppType}}::Init {
    ChainedStruct const * nextInChain;
    {% for member in type.members if member.type.category != "callback info" %}
        {% set member_declaration = as_annotated_cppType(member, type.has_free_members_function) + render_cpp_default_value(member, True, type.has_free_members_function) %}
        {{member_declaration}};
    {% endfor %}
};

{{CppType}}::{{CppType}}({{CppType}}::Init&& init) : detail::{{CppType}} {
    init.nextInChain
    {%- for member in type.members if member.type.category != "callback info" -%},{{" "}}
        std::move(init.{{as_varName(member.name)}})
    {%- endfor -%}
} {}

static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

template <typename F, typename T, typename Cb, typename>
void {{CppType}}::SetDeviceLostCallback(CallbackMode callbackMode, F callback, T userdata) {
    assert(deviceLostCallbackInfo2.callback == nullptr);

    deviceLostCallbackInfo2.mode = static_cast<WGPUCallbackMode>(callbackMode);
    deviceLostCallbackInfo2.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, char const * message, void* callback, void* userdata) {
        auto cb = reinterpret_cast<Cb*>(callback);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<DeviceLostReason>(reason), message, static_cast<T>(userdata));
        apiDevice.MoveToCHandle();
    };
    deviceLostCallbackInfo2.userdata1 = reinterpret_cast<void*>(+callback);
    deviceLostCallbackInfo2.userdata2 = reinterpret_cast<void*>(userdata);
}

template <typename L, typename Cb, typename>
void {{CppType}}::SetDeviceLostCallback(CallbackMode callbackMode, L callback) {
    assert(deviceLostCallbackInfo2.callback == nullptr);
    using F = void (const Device& device, DeviceLostReason reason, const char * message);

    deviceLostCallbackInfo2.mode = static_cast<WGPUCallbackMode>(callbackMode);
    if constexpr (std::is_convertible_v<L, F*>) {
        deviceLostCallbackInfo2.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, char const * message, void* callback, void*) {
            auto cb = reinterpret_cast<F*>(callback);
            // We manually acquire and release the device to avoid changing any ref counts.
            auto apiDevice = Device::Acquire(*device);
            (*cb)(apiDevice, static_cast<DeviceLostReason>(reason), message);
            apiDevice.MoveToCHandle();
        };
        deviceLostCallbackInfo2.userdata1 = reinterpret_cast<void*>(+callback);
        deviceLostCallbackInfo2.userdata2 = nullptr;
    } else {
        auto* lambda = new L(std::move(callback));
        deviceLostCallbackInfo2.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, char const * message, void* callback, void*) {
            std::unique_ptr<L> lambda(reinterpret_cast<L*>(callback));
            // We manually acquire and release the device to avoid changing any ref counts.
            auto apiDevice = Device::Acquire(*device);
            (*lambda)(apiDevice, static_cast<DeviceLostReason>(reason), message);
            apiDevice.MoveToCHandle();
        };
        deviceLostCallbackInfo2.userdata1 = reinterpret_cast<void*>(lambda);
        deviceLostCallbackInfo2.userdata2 = nullptr;
    }
}

template <typename F, typename T, typename Cb, typename>
void {{CppType}}::SetUncapturedErrorCallback(F callback, T userdata) {
    uncapturedErrorCallbackInfo2.callback = [](WGPUDevice const * device, WGPUErrorType type, char const * message, void* callback, void* userdata) {
        auto cb = reinterpret_cast<Cb*>(callback);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<ErrorType>(type), message, static_cast<T>(userdata));
        apiDevice.MoveToCHandle();
    };
    uncapturedErrorCallbackInfo2.userdata1 = reinterpret_cast<void*>(+callback);
    uncapturedErrorCallbackInfo2.userdata2 = reinterpret_cast<void*>(userdata);
}

template <typename L, typename Cb, typename>
void {{CppType}}::SetUncapturedErrorCallback(L callback) {
    using F = void (const Device& device, ErrorType type, const char * message);
    static_assert(std::is_convertible_v<L, F*>, "Uncaptured error callback cannot be a binding lambda");

    uncapturedErrorCallbackInfo2.callback = [](WGPUDevice const * device, WGPUErrorType type, char const * message, void* callback, void*) {
        auto cb = reinterpret_cast<F*>(callback);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<ErrorType>(type), message);
        apiDevice.MoveToCHandle();
    };
    uncapturedErrorCallbackInfo2.userdata1 = reinterpret_cast<void*>(+callback);
    uncapturedErrorCallbackInfo2.userdata2 = nullptr;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

{% for type in by_category["object"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    // {{CppType}} implementation

    {% for method in type.methods %}
        {% if has_callbackInfoStruct(method) %}
            {{render_cpp_callback_info_template_method_impl(type, method)}}
            {{render_cpp_callback_info_lambda_method_impl(type, method)}}
        {% else %}
            {{render_cpp_method_impl(type, method)}}
        {% endif %}
    {% endfor %}

    {% if CppType == "Instance" %}
        WaitStatus Instance::WaitAny(Future f, uint64_t timeout) const {
            FutureWaitInfo waitInfo { f };
            return WaitAny(1, &waitInfo, timeout);
        }
    {% endif %}

    void {{CppType}}::{{c_prefix}}AddRef({{CType}} handle) {
        if (handle != nullptr) {
            {{as_cMethodNamespaced(type.name, Name("add ref"), c_namespace)}}(handle);
        }
    }
    void {{CppType}}::{{c_prefix}}Release({{CType}} handle) {
        if (handle != nullptr) {
            {{as_cMethodNamespaced(type.name, Name("release"), c_namespace)}}(handle);
        }
    }
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

{% if c_namespace %}
    }  // namespace {{c_namespace.namespace_case()}}

    {% for type in by_category["object"] %}
        using {{as_cppType(type.name)}} = {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] %}
        using {{as_cppType(type.name)}} = {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}};
    {% endfor %}
{% endif %}

{% for typeDef in by_category["typedef"] %}
    // {{as_cppType(typeDef.name)}} is deprecated.
    // Use {{as_cppType(typeDef.type.name)}} instead.
    using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};
{% endfor %}

// Free Functions
{% for function in by_category["function"] if not function.no_cpp %}
    //* TODO(crbug.com/42241188): Remove "2" suffix when WGPUStringView changes complete.
    {% set OriginalFunctionName = as_cppType(function.name) %}
    {% set FunctionName = OriginalFunctionName[:-1] if function.name.chunks[-1] == "2" else OriginalFunctionName %}
    static inline {{as_cppType(function.return_type.name)}} {{FunctionName}}(
        {%- for arg in function.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {{as_annotated_cppType(arg)}}{{render_cpp_default_value(arg, False)}}
        {%- endfor -%}
    ) {
        {% if function.return_type.name.concatcase() == "void" %}
            {{render_function_call(function)}};
        {% else %}
            auto result = {{render_function_call(function)}};
            return {{convert_cType_to_cppType(function.return_type, 'value', 'result')}};
        {% endif %}
    }
{% endfor %}

}  // namespace {{metadata.namespace}}

namespace wgpu {
{% for type in by_category["bitmask"] %}
    template<>
    struct IsWGPUBitmask<{{metadata.namespace}}::{{as_cppType(type.name)}}> {
        static constexpr bool enable = true;
    };

{% endfor %}
} // namespace wgpu

namespace std {
// Custom boolean class needs corresponding hash function so that it appears as a transparent bool.
template <>
struct hash<{{metadata.namespace}}::{{BoolCppType}}> {
  public:
    size_t operator()(const {{metadata.namespace}}::{{BoolCppType}} &v) const {
        return hash<bool>()(v);
    }
};
template <>
struct hash<{{metadata.namespace}}::{{OptionalBoolCppType}}> {
  public:
    size_t operator()(const {{metadata.namespace}}::{{OptionalBoolCppType}} &v) const {
        return hash<{{OptionalBoolCType}}>()(v.mValue);
    }
};
}  // namespace std

#endif // {{PREFIX}}{{API}}_CPP_H_
