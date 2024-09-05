#version 310 es


struct S {
  ivec3 v;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v_1;
void f() {
  v_1.tint_symbol.v = ivec3(1, 2, 3);
  v_1.tint_symbol.v[0u] = 1;
  v_1.tint_symbol.v[1u] = 2;
  v_1.tint_symbol.v[2u] = 3;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
