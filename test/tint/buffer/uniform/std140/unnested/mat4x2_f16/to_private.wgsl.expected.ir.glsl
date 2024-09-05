#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  f16vec2 tint_symbol_col0;
  f16vec2 tint_symbol_col1;
  f16vec2 tint_symbol_col2;
  f16vec2 tint_symbol_col3;
} v;
f16mat4x2 p = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = f16mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3);
  p[1] = f16mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0];
  p[1] = f16mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0].yx;
  p[0][1] = f16mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[1][0];
}
