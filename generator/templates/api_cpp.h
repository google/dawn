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
{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}
{% set PREFIX = "" if not c_namespace else c_namespace.SNAKE_CASE() + "_" %}
#ifndef {{PREFIX}}{{API}}_CPP_H_
#define {{PREFIX}}{{API}}_CPP_H_

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>

#include "{{c_header}}"
#include "{{api}}/{{api}}_cpp_chained_struct.h"
#include "{{api}}/{{api}}_enum_class_bitmasks.h"

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

{% for type in by_category["enum"] %}
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
    {% set CType = as_cType(type.name) + "Flags" %}
    enum class {{CppType}} : uint32_t {
        {% for value in type.values %}
            {{as_cppEnum(value.name)}} = {{as_cEnum(type.name, value.name)}},
        {% endfor %}
    };
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

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
    {% set CallbackType = (CallbackInfoType.members|first).type %}
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
    {{as_cppType(types["callback mode"].name)}} mode, F callback, T userdata) const
{%- endmacro %}

//* This rendering macro should NOT be used for callback info type functions.
{% macro render_cpp_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set OriginalMethodName = method.name.CamelCase() %}
    {% set MethodName = OriginalMethodName[:-1] if method.name.chunks[-1] == "f" else OriginalMethodName %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {{as_cppType(method.return_type.name)}} {{MethodName}}(
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
        {% set CallbackType = (CallbackInfoType.members|first).type %}
        {{as_cType(CallbackInfoType.name)}} callbackInfo = {};
        callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(mode);
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
        auto result = {{as_cMethod(type.name, method.name)}}(Get(){{", "}}
            {%- for arg in method.arguments if arg.type.category != "callback info" -%}{{render_c_actual_arg(arg)}}{{", "}}
            {%- endfor -%}
        callbackInfo);
        return {{convert_cType_to_cppType(method.return_type, 'value', 'result') | indent(8)}};
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
            {% else %}
                inline {{render_cpp_method_declaration(type, method)}};
            {% endif %}
        {% endfor %}

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

{% for type in by_category["structure"] %}
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
        {% if type.has_free_members_function %}

          private:
            static inline void Reset({{as_cppType(type.name)}}& value);
        {% endif %}
    };

{% endfor %}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
// error: 'offsetof' within non-standard-layout type '{{metadata.namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
{% for type in by_category["structure"] %}
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
            if (
                {%- for member in type.members if member.annotation != 'value' %}
                    {% if not loop.first %} || {% endif -%}
                    this->{{member.name.camelCase()}} != nullptr
                {%- endfor -%}
            ) {
                {{as_cMethodNamespaced(type.name, Name("free members"), c_namespace)}}(
                    *reinterpret_cast<{{as_cType(type.name)}}*>(this));
            }
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
            this->~{{CppType}}();
            {% for member in type.members %}
                detail::AsNonConstReference(this->{{member.name.camelCase()}}) = std::move(rhs.{{member.name.camelCase()}});
            {% endfor %}
            Reset(rhs);
            return *this;
        }

            // static
        void {{CppType}}::Reset({{CppType}}& value) {
            {{CppType}} defaultValue{};
            {% for member in type.members %}
                detail::AsNonConstReference(value.{{member.name.camelCase()}}) = defaultValue.{{member.name.camelCase()}};
            {% endfor %}
        }
    {% endif %}

    {{CppType}}::operator const {{as_cType(type.name)}}&() const noexcept {
        return *reinterpret_cast<const {{as_cType(type.name)}}*>(this);
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
        {% else %}
            {{render_cpp_method_impl(type, method)}}
        {% endif %}
    {% endfor %}
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
    static inline {{as_cppType(function.return_type.name)}} {{as_cppType(function.name)}}(
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
}  // namespace std

#endif // {{PREFIX}}{{API}}_CPP_H_
