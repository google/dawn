#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat2x4 tint_symbol[4];
} v_1;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float tint_symbol_2;
} v_2;
float a(mat2x4 a_1[4]) {
  return a_1[0][0][0u];
}
float b(mat2x4 m) {
  return m[0][0u];
}
float c(vec4 v) {
  return v[0u];
}
float d(float f) {
  return f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_3 = a(v_1.tint_symbol);
  float v_4 = (v_3 + b(v_1.tint_symbol[1]));
  float v_5 = (v_4 + c(v_1.tint_symbol[1][0].ywxz));
  v_2.tint_symbol_2 = (v_5 + d(v_1.tint_symbol[1][0].ywxz[0u]));
}
