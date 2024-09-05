#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat3x4 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float tint_symbol_2;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x3 t = transpose(v.tint_symbol[2]);
  float l = length(v.tint_symbol[0][1].ywxz);
  float a = abs(v.tint_symbol[0][1].ywxz[0u]);
  float v_2 = (t[0][0u] + float(l));
  v_1.tint_symbol_2 = (v_2 + float(a));
}
