#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16mat2x4 tint_symbol;
f16mat2x4 tint_symbol_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1 = tint_symbol;
}
