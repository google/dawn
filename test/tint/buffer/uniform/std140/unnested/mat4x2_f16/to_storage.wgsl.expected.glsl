#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat4x2 inner;
} v_1;
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  f16vec2 v_5 = tint_bitcast_to_16bit(v.inner[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  f16vec2 v_7 = tint_bitcast_to_16bit(v.inner[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return f16mat4x2(v_3, v_5, v_7, tint_bitcast_to_16bit(v.inner[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = v_2(0u);
  v_1.inner[1u] = tint_bitcast_to_16bit(v.inner[0u].x);
  v_1.inner[1u] = tint_bitcast_to_16bit(v.inner[0u].x).yx;
  uvec4 v_9 = v.inner[0u];
  v_1.inner[0u].y = tint_bitcast_to_16bit(v_9.y).x;
}
