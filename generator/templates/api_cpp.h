//* Copyright 2017 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}
#ifndef {{API}}_CPP_H_
#define {{API}}_CPP_H_

#include "dawn/{{api}}.h"
#include "dawn/{{api}}_cpp_chained_struct.h"
#include "dawn/EnumClassBitmasks.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace {{metadata.namespace}} {

    namespace detail {
        constexpr size_t ConstexprMax(size_t a, size_t b) {
            return a > b ? a : b;
        }
    }  // namespace detail

    {% set c_prefix = metadata.c_prefix %}
    {% for constant in by_category["constant"] %}
        {% set type = as_cppType(constant.type.name) %}
        {% set value = c_prefix + "_" +  constant.name.SNAKE_CASE() %}
        static constexpr {{type}} k{{as_cppType(constant.name)}} = {{ value }};
    {% endfor %}

    {% for type in by_category["enum"] %}
        enum class {{as_cppType(type.name)}} : uint32_t {
            {% for value in type.values %}
                {{as_cppEnum(value.name)}} = 0x{{format(value.value, "08X")}},
            {% endfor %}
        };

    {% endfor %}

    {% for type in by_category["bitmask"] %}
        enum class {{as_cppType(type.name)}} : uint32_t {
            {% for value in type.values %}
                {{as_cppEnum(value.name)}} = 0x{{format(value.value, "08X")}},
            {% endfor %}
        };

    {% endfor %}

    {% for type in by_category["function pointer"] %}
        using {{as_cppType(type.name)}} = {{as_cType(type.name)}};
    {% endfor %}

    {% for type in by_category["object"] %}
        class {{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] %}
        struct {{as_cppType(type.name)}};
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

    {% for typeDef in by_category["typedef"] %}
        // {{as_cppType(typeDef.name)}} is deprecated.
        // Use {{as_cppType(typeDef.type.name)}} instead.
        using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};

    {% endfor %}
    template<typename Derived, typename CType>
    class ObjectBase {
      public:
        ObjectBase() = default;
        ObjectBase(CType handle): mHandle(handle) {
            if (mHandle) Derived::{{c_prefix}}Reference(mHandle);
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
                if (mHandle) Derived::{{c_prefix}}Reference(mHandle);
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

{% macro render_cpp_default_value(member, is_struct=True, force_default=False) -%}
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

{% macro render_cpp_method_declaration(type, method) %}
    {% set CppType = as_cppType(type.name) %}
    {{as_cppType(method.return_type.name)}} {{method.name.CamelCase()}}(
        {%- for arg in method.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}
            {%- endif -%}
            {{render_cpp_default_value(arg, False)}}
        {%- endfor -%}
    ) const
{%- endmacro %}

    {% for type in by_category["object"] %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}
        class {{CppType}} : public ObjectBase<{{CppType}}, {{CType}}> {
          public:
            using ObjectBase::ObjectBase;
            using ObjectBase::operator=;

            {% for method in type.methods %}
                {{render_cpp_method_declaration(type, method)}};
            {% endfor %}

          private:
            friend ObjectBase<{{CppType}}, {{CType}}>;
            static void {{c_prefix}}Reference({{CType}} handle);
            static void {{c_prefix}}Release({{CType}} handle);
        };

    {% endfor %}

    {% for function in by_category["function"] if not function.no_cpp %}
        {{as_cppType(function.return_type.name)}} {{as_cppType(function.name)}}(
            {%- for arg in function.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{as_annotated_cppType(arg)}}{{render_cpp_default_value(arg, False)}}
            {%- endfor -%}
        );
    {% endfor %}

    {% for type in by_category["structure"] %}
        {% set Out = "Out" if type.output else "" %}
        {% set const = "const" if not type.output else "" %}
        {% if type.chained %}
            {% for root in type.chain_roots %}
                // Can be chained in {{as_cppType(root.name)}}
            {% endfor %}
            struct {{as_cppType(type.name)}} : ChainedStruct{{Out}} {
                {{as_cppType(type.name)}}() {
                    sType = SType::{{type.name.CamelCase()}};
                }
        {% else %}
            struct {{as_cppType(type.name)}} {
        {% endif %}
            {% if type.has_free_members_function %}
                {{as_cppType(type.name)}}() = default;
                ~{{as_cppType(type.name)}}();
                {{as_cppType(type.name)}}(const {{as_cppType(type.name)}}&) = delete;
                {{as_cppType(type.name)}}& operator=(const {{as_cppType(type.name)}}&) = delete;
                {{as_cppType(type.name)}}({{as_cppType(type.name)}}&&);
                {{as_cppType(type.name)}}& operator=({{as_cppType(type.name)}}&&);
            {% endif %}
            {% if type.extensible %}
                ChainedStruct{{Out}} {{const}} * nextInChain = nullptr;
            {% endif %}
            {% for member in type.members %}
                {% set member_declaration = as_annotated_cppType(member, type.has_free_members_function) + render_cpp_default_value(member, False, type.has_free_members_function) %}
                {% if type.chained and loop.first %}
                    //* Align the first member after ChainedStruct to match the C struct layout.
                    //* It has to be aligned both to its natural and ChainedStruct's alignment.
                    static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct{{out}}), alignof({{decorate("", as_cppType(member.type.name), member)}}));
                    alignas(kFirstMemberAlignment) {{member_declaration}};
                {% else %}
                    {{member_declaration}};
                {% endif %}
            {% endfor %}
        };

    {% endfor %}

    // The operators of EnumClassBitmmasks in the dawn:: namespace need to be imported
    // in the {{metadata.namespace}} namespace for Argument Dependent Lookup.
    DAWN_IMPORT_BITMASK_OPERATORS
}  // namespace {{metadata.namespace}}

namespace dawn {
    {% for type in by_category["bitmask"] %}
        template<>
        struct IsDawnBitmask<{{metadata.namespace}}::{{as_cppType(type.name)}}> {
            static constexpr bool enable = true;
        };

    {% endfor %}
} // namespace dawn

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

#endif // {{API}}_CPP_H_
