#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  f16vec3 tint_symbol_col0;
  f16vec3 tint_symbol_col1;
  f16vec3 tint_symbol_col2;
} v;
f16mat3 p = f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = f16mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2);
  p[1] = f16mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2)[0];
  p[1] = f16mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2)[0].zxy;
  p[0][1] = f16mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2)[1][0];
}
