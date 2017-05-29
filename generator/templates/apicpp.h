//* Copyright 2017 The NXT Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

#ifndef NXTCPP_H
#define NXTCPP_H

#include "nxt.h"

#include "nxt/EnumClassBitmasks.h"

namespace nxt {

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

    {% for type in by_category["bitmask"] %}
        template<>
        struct IsNXTBitmask<{{as_cppType(type.name)}}> {
            static constexpr bool enable = true;
        };

    {% endfor %}

    {% for type in by_category["natively defined"] %}
        using {{as_cppType(type.name)}} = {{as_cType(type.name)}};
    {% endfor %}

    {% for type in by_category["object"] %}
        class {{as_cppType(type.name)}};
    {% endfor %}

    template<typename Derived, typename CType>
    class ObjectBase {
        public:
            ObjectBase() = default;
            ObjectBase(CType handle): handle(handle) {
                if (handle) Derived::NxtReference(handle);
            }
            ~ObjectBase() {
                if (handle) Derived::NxtRelease(handle);
            }

            ObjectBase(ObjectBase const& other) = delete;
            Derived& operator=(ObjectBase const& other) = delete;

            ObjectBase(ObjectBase&& other) {
                handle = other.handle;
                other.handle = 0;
            }
            Derived& operator=(ObjectBase&& other) {
                if (&other == this) return static_cast<Derived&>(*this);

                if (handle) Derived::NxtRelease(handle);
                handle = other.handle;
                other.handle = 0;

                return static_cast<Derived&>(*this);
            }

            explicit operator bool() const {
                return handle != nullptr;
            }
            CType Get() const {
                return handle;
            }
            CType Release() {
                CType result = handle;
                handle = 0;
                return result;
            }
            Derived Clone() const {
                return Derived(handle);
            }
            static Derived Acquire(CType handle) {
                Derived result;
                result.handle = handle;
                return result;
            }

        protected:
            CType handle = nullptr;
    };

    {% macro render_cpp_method_declaration(type, method) %}
        {% set CppType = as_cppType(type.name) %}
        {% if method.return_type.name.concatcase() == "void" -%}
            {{CppType}} const&
        {%- else -%}
            {{as_cppType(method.return_type.name)}}
        {%- endif -%}
        {{" "}}{{method.name.CamelCase()}}(
            {%- for arg in method.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                    {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}
                {%- else -%}
                    {{as_annotated_cppType(arg)}}
                {%- endif -%}
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

                {% for method in native_methods(type) %}
                    {{render_cpp_method_declaration(type, method)}};
                {% endfor %}

            private:
                friend ObjectBase<{{CppType}}, {{CType}}>;
                static void NxtReference({{CType}} handle);
                static void NxtRelease({{CType}} handle);
        };

    {% endfor %}

} // namespace nxt

#endif // NXTCPP_H
