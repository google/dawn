#include <cstdio>
#include <webgpu/webgpu.h>

int main() {
    {% for function in by_category["function"] %}
        printf("%p\n", {{as_cMethod(None, function.name)}});
    {% endfor %}
    {% for type in by_category["object"] if len(c_methods(type)) > 0 %}

        {% for method in c_methods(type) %}
            printf("%p\n", {{as_cMethod(type.name, method.name)}});
        {% endfor %}
    {% endfor %}
}
