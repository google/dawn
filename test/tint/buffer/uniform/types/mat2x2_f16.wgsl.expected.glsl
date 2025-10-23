#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat2 inner;
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_3, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2 x = v_2(0u);
  v_1.inner = x;
}
