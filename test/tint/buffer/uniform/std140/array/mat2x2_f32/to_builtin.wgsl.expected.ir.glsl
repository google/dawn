#version 310 es


struct mat2x2_f32_std140 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  mat2x2_f32_std140 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float tint_symbol_2;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 t = transpose(mat2(v.tint_symbol[2].col0, v.tint_symbol[2].col1));
  float l = length(v.tint_symbol[0].col1.yx);
  float a = abs(v.tint_symbol[0].col1.yx[0u]);
  float v_2 = (t[0][0u] + float(l));
  v_1.tint_symbol_2 = (v_2 + float(a));
}
