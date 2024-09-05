#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S_std140 {
  int before;
  f16vec2 m_col0;
  f16vec2 m_col1;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2 t = transpose(f16mat2(v.tint_symbol[2].m_col0, v.tint_symbol[2].m_col1));
  float16_t l = length(v.tint_symbol[0].m_col1.yx);
  float16_t a = abs(v.tint_symbol[0].m_col1.yx[0u]);
}
