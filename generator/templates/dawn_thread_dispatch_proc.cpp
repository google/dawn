#include "dawn/dawn_thread_dispatch_proc.h"

#include <thread>

static DawnProcTable nullProcs;
thread_local DawnProcTable perThreadProcs;

void dawnProcSetPerThreadProcs(const DawnProcTable* procs) {
    if (procs) {
        perThreadProcs = *procs;
    } else {
        perThreadProcs = nullProcs;
    }
}

static WGPUProc ThreadDispatchGetProcAddress(WGPUDevice device, const char* procName) {
    return perThreadProcs.getProcAddress(device, procName);
}

static WGPUInstance ThreadDispatchCreateInstance(WGPUInstanceDescriptor const * descriptor) {
    return perThreadProcs.createInstance(descriptor);
}

{% for type in by_category["object"] %}
    {% for method in c_methods(type) %}
        static {{as_cType(method.return_type.name)}} ThreadDispatch{{as_MethodSuffix(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% if method.return_type.name.canonical_case() != "void" %}return {% endif %}
            perThreadProcs.{{as_varName(type.name, method.name)}}({{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }
    {% endfor %}
{% endfor %}

extern "C" {
    DawnProcTable dawnThreadDispatchProcTable = {
        ThreadDispatchGetProcAddress,
        ThreadDispatchCreateInstance,
{% for type in by_category["object"] %}
    {% for method in c_methods(type) %}
        ThreadDispatch{{as_MethodSuffix(type.name, method.name)}},
    {% endfor %}
{% endfor %}
    };
}
