#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat2x4 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  mat2x4 tint_symbol_2[4];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.tint_symbol_2 = v.tint_symbol;
  v_1.tint_symbol_2[1] = v.tint_symbol[2];
  v_1.tint_symbol_2[1][0] = v.tint_symbol[0][1].ywxz;
  v_1.tint_symbol_2[1][0][0u] = v.tint_symbol[0][1].x;
}
