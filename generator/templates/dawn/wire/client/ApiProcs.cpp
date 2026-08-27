//* Copyright 2019 The Dawn & Tint Authors
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

{% from 'dawn/cpp_macros.tmpl' import as_dawnType with context %}
{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "dawn/wire/client/{{prefix}}_platform.h"
#include "dawn/wire/client/webgpu.h"
#include "dawn/dawn_version.h"
#include "src/dawn/wire/client/Client.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"

{% from 'dawn/cpp_macros.tmpl' import convert_arguments_and_call with context %}

namespace dawn::wire::client {

    // Template function for constexpr branching when creating new objects.
    template <typename Parent, typename Child, typename... Args>
    Child* Create(Parent p, Args... args) {
        if constexpr (std::is_constructible_v<Child, const ObjectBaseParams&, decltype(args)...>) {
            return p->GetClient()->template Make<Child>(args...).Detach();
        } else if constexpr (std::is_constructible_v<Child, const ObjectBaseParams&, Ref<Instance>, decltype(args)...>) {
            return p->GetClient()->template Make<Child>(p->GetInstance(), args...).Detach();
        } else {
            if constexpr (std::is_base_of_v<ObjectWithEventsBase, Child>) {
                return p->GetClient()->template Make<Child>(p->GetInstance()).Detach();
            } else {
                return p->GetClient()->template Make<Child>().Detach();
            }
        }
    }

}  // namespace dawn::wire::client

namespace {
// Alias the dawn::wire::client namespace in because we need a lot of the helpers, structures, and
// types, and currently the client wgpu* implementations need to be in global namespace. This is
// ok because this is an implementation file, and we alias the namespace into an anonymous
// namespace in this case.
using namespace dawn::wire::client;
}  // namespace

//* Implementation of the client API functions.
{% for (type, methods) in c_methods_sorted_by_parent %}
    {%- set Type = type.name.CamelCase() -%}
    {%- set cType = as_cType(type.name) -%}

    {% for method in methods %}
        {% set Suffix = as_MethodSuffix(type.name, method.name) %}

        DAWN_WIRE_EXPORT {{as_annotated_cType(method.returns)}} {{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}}(
            {{-cType}} cSelf
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% if Suffix not in client_handwritten_commands %}
                auto self = FromAPI(cSelf);
                dawn::wire::{{Suffix}}Cmd cmd;

                //* Create the structure going on the wire on the stack and fill it with the value
                //* arguments so it can compute its size.
                cmd.self = cSelf;

                //* For object creation, store the object ID the client will use for the result.
                {% if method.returns and method.returns.type.category == "object" %}
                    {% set ReturnObj = method.returns.type.name.CamelCase() %}
                    {{ReturnObj}}* returnObject = Create<{{as_wireType(type)}}, {{ReturnObj}}>(self
                        {%- for arg in method.arguments -%}
                                , {{as_varName(arg.name)}}
                        {%- endfor -%}
                    );
                    cmd.result = returnObject->GetWireHandle(self->GetClient());
                {% endif %}

                {% for arg in method.arguments %}
                    //* Commands with mutable pointers should not be autogenerated.
                    {{assert(arg.annotation != "*")}}
                    {% set varName = as_varName(arg.name) %}
                    {% if arg.is_length %}
                        //* Skipped as it is included in the span below.
                    {% elif arg.length and arg.constant_length != 1 %}
                        {% if arg.type.name.get() == "void" %}
                            {% set raw_type = "std::byte" %}
                        {% elif arg.type.category == "object" %}
                            {% set raw_type = as_cType(arg.type.name) %}
                        {% else %}
                            {% set raw_type = as_dawnType(arg.type) %}
                        {% endif %}
                        using {{varName}}SpanT = std::remove_pointer_t<{{decorate(raw_type, arg)}}>;
                        auto* {{varName}}Ptr = reinterpret_cast<{{varName}}SpanT*>(dawn::wire::FromAPI({{varName}}));
                        {% if arg.length == "constant" %}
                            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
                            cmd.{{varName}} = DAWN_UNSAFE_BUFFERS(dawn::Span<{{varName}}SpanT, {{arg.constant_length}}>({{varName}}Ptr));
                        {% else %}
                            size_t {{varName}}SizeV = dawn::checked_cast<size_t>({{as_varName(arg.length.name)}});
                            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
                            cmd.{{varName}} = DAWN_UNSAFE_BUFFERS(dawn::Span<{{varName}}SpanT>({{varName}}Ptr, {{varName}}SizeV));
                        {% endif %}
                    {% else %}
                        cmd.{{varName}} = dawn::wire::FromAPI({{varName}});
                    {% endif %}
                {% endfor %}

                //* Allocate space to send the command and copy the value args over.
                self->GetClient()->SerializeCommand(std::move(cmd));

                {% if method.returns and method.returns.type.category == "object" %}
                    return ToAPI(returnObject);
                {% endif %}
            {% elif type.category == "object" %}
                auto self = FromAPI(cSelf);
                {{convert_arguments_and_call(method, "self->API" + method.name.CamelCase())}}
            {% elif type.category == "structure" %}
                return API{{Suffix}}(cSelf);
            {% endif %}
        }
    {% endfor %}

{% endfor %}

namespace {
    struct ProcEntry {
        WGPUProc proc;
        std::string_view name;
    };
    static const ProcEntry sProcMap[] = {
        {% for (type, method) in c_methods_sorted_by_name %}
            { reinterpret_cast<WGPUProc>({{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}}), "{{as_cMethod(type.name, method.name)}}" },
        {% endfor %}
    };
    static constexpr size_t sProcMapSize = sizeof(sProcMap) / sizeof(sProcMap[0]);
}  // anonymous namespace

DAWN_WIRE_EXPORT WGPUProc {{as_cMethodNamespaced(None, Name('get proc address'), Name('dawn wire client'))}}(WGPUStringView cProcName) {
    if (cProcName.data == nullptr) {
        return nullptr;
    }

    std::string_view procName(cProcName.data, cProcName.length != WGPU_STRLEN ? cProcName.length : strlen(cProcName.data));

    const ProcEntry* entry = std::lower_bound(&sProcMap[0], &sProcMap[sProcMapSize], procName,
        [](const ProcEntry &a, const std::string_view& b) -> bool {
            return a.name.compare(b) < 0;
        }
    );

    if (entry != &sProcMap[sProcMapSize] && entry->name == procName) {
        return entry->proc;
    }

    // Special case the free-standing functions of the API.
    // TODO(dawn:1238) Checking string one by one is slow, it needs to be optimized.
    {% for function in by_category["function"] %}
        if (procName == "{{as_cMethod(None, function.name)}}") {
            return reinterpret_cast<WGPUProc>({{as_cMethodNamespaced(None, function.name, Name('dawn wire client'))}});
        }

    {% endfor %}
    return nullptr;
}

namespace dawn::wire::client {

    std::vector<std::string_view> GetProcMapNamesForTesting() {
        std::vector<std::string_view> result;
        result.reserve(sProcMapSize);
        for (const ProcEntry& entry : sProcMap) {
            result.push_back(entry.name);
        }
        return result;
    }

    {% set Prefix = metadata.proc_table_prefix %}

    constexpr {{Prefix}}ProcTable MakeProcTable() {
        {{Prefix}}ProcTable procs = {};
        std::ranges::copy(dawn::kDawnVersion, procs.version);
        {% for function in by_category["function"] %}
            procs.{{as_varName(function.name)}} = {{as_cMethodNamespaced(None, function.name, Name('dawn wire client'))}};
        {% endfor %}
        {% for (type, methods) in c_methods_sorted_by_parent %}
            {% for method in methods %}
                procs.{{as_varName(type.name, method.name)}} = {{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}};
            {% endfor %}
        {% endfor %}
        return procs;
    }

    static {{Prefix}}ProcTable gProcTable = MakeProcTable();

    const {{Prefix}}ProcTable& GetProcs() {
        return gProcTable;
    }

}  // namespace dawn::wire::client
