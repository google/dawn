#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16vec2 inner;
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec2 x = tint_bitcast_to_f16(v.inner[0u].x);
  v_1.inner = x;
}
