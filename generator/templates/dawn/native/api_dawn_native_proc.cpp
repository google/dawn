// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/{{metadata.api.lower()}}.h"

namespace dawn::native {

// This file should be kept in sync with generator/templates/dawn/native/ProcTable.cpp

{% for function in by_category["function"] %}
    extern {{as_cType(function.return_type.name)}} Native{{as_cppType(function.name)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
        {%- endfor -%}
    );
{% endfor %}
{% for type in by_category["object"] %}
    {% for method in c_methods(type) %}
        extern {{as_cType(method.return_type.name)}} Native{{as_MethodSuffix(type.name, method.name)}}(
            {{-as_cType(type.name)}} cSelf
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}
{% endfor %}

}

extern "C" {
    using namespace dawn::native;

    {% for function in by_category["function"] %}
        {{as_cType(function.return_type.name)}} {{metadata.namespace}}{{as_cppType(function.name)}} (
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            return Native{{as_cppType(function.name)}}(
                {%- for arg in function.arguments -%}
                    {% if not loop.first %}, {% endif %}{{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }
    {% endfor %}

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_cType(method.return_type.name)}} {{metadata.namespace}}{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                return Native{{as_MethodSuffix(type.name, method.name)}}(
                    cSelf
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}
    {% endfor %}
}
