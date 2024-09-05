#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_4_1_ssbo {
  mat4 tint_symbol_3;
} v;
layout(binding = 1, std430)
buffer tint_symbol_6_1_ssbo {
  mat4 tint_symbol_5;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.tint_symbol_5 = v.tint_symbol_3;
}
