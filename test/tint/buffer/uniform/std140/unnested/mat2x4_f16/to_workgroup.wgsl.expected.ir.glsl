#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  f16vec4 tint_symbol_col0;
  f16vec4 tint_symbol_col1;
} v;
shared f16mat2x4 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    w = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
  }
  barrier();
  w = f16mat2x4(v.tint_symbol_col0, v.tint_symbol_col1);
  w[1] = f16mat2x4(v.tint_symbol_col0, v.tint_symbol_col1)[0];
  w[1] = f16mat2x4(v.tint_symbol_col0, v.tint_symbol_col1)[0].ywxz;
  w[0][1] = f16mat2x4(v.tint_symbol_col0, v.tint_symbol_col1)[1][0];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
