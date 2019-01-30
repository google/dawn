//* Copyright 2019 The Dawn Authors
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

// Forwarding callbacks
{% for type in by_category["object"] if type.is_builder%}
    static void Forward{{type.name.CamelCase()}}ToClient(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2);
{% endfor %}

// Error callbacks
{% for type in by_category["object"] if type.is_builder%}
    {% set Type = type.name.CamelCase() %}
    void On{{Type}}Error(dawnBuilderErrorStatus status, const char* message, uint32_t id, uint32_t serial);
{% endfor %}

// Command handlers
{% for type in by_category["object"] %}
    {% for method in type.methods %}
        {% set Suffix = as_MethodSuffix(type.name, method.name) %}
        {% if Suffix not in client_side_commands %}
            bool Handle{{Suffix}}(const char** commands, size_t* size);
        {% endif %}
    {% endfor %}
{% endfor %}
