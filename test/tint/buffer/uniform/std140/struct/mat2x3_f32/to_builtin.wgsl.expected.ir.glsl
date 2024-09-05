#version 310 es


struct S_std140 {
  int before;
  vec3 m_col0;
  vec3 m_col1;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x2 t = transpose(mat2x3(v.tint_symbol[2].m_col0, v.tint_symbol[2].m_col1));
  float l = length(v.tint_symbol[0].m_col1.zxy);
  float a = abs(v.tint_symbol[0].m_col1.zxy[0u]);
}
