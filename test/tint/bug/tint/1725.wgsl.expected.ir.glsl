#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_8_1_ssbo {
  uint tint_symbol_7[];
} v;
void tint_symbol_1_inner(uint tint_symbol_2) {
  int tint_symbol_3 = 0;
  int tint_symbol_4 = 0;
  int tint_symbol_5 = 0;
  uint v_1 = min(tint_symbol_2, (uint(v.tint_symbol_7.length()) - 1u));
  uint tint_symbol_6 = v.tint_symbol_7[v_1];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_LocalInvocationIndex);
}
