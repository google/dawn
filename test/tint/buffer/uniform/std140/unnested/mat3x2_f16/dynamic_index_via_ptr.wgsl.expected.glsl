#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[1];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  return f16mat3x2(v_3, v_4, tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_5 = (4u * min(uint(i()), 2u));
  f16mat3x2 l_m = v_2(0u);
  f16vec2 l_m_i = tint_bitcast_to_f16(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
}
