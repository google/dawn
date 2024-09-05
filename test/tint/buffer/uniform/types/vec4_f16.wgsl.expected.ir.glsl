#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  f16vec4 tint_symbol_1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  f16vec4 tint_symbol_3;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec4 x = v.tint_symbol_1;
  v_1.tint_symbol_3 = x;
}
