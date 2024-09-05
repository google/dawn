#version 310 es


struct S {
  uvec3 v;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v_1;
void f() {
  v_1.tint_symbol.v = uvec3(1u, 2u, 3u);
  v_1.tint_symbol.v[0u] = 1u;
  v_1.tint_symbol.v[1u] = 2u;
  v_1.tint_symbol.v[2u] = 3u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
