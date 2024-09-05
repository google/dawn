#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  f16vec4 tint_symbol_col0;
  f16vec4 tint_symbol_col1;
  f16vec4 tint_symbol_col2;
  f16vec4 tint_symbol_col3;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4 t = transpose(f16mat4(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3));
  float16_t l = length(f16mat4(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[1]);
  float16_t a = abs(f16mat4(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0].ywxz[0u]);
}
