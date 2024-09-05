#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat3x4 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float tint_symbol_2;
} v_1;
mat3x4 p[4] = mat3x4[4](mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.tint_symbol;
  p[1] = v.tint_symbol[2];
  p[1][0] = v.tint_symbol[0][1].ywxz;
  p[1][0][0u] = v.tint_symbol[0][1].x;
  v_1.tint_symbol_2 = p[1][0].x;
}
