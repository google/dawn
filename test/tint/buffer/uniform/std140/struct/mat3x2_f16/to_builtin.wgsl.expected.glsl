#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  return f16mat3x2(v_2, v_4, tint_bitcast_to_16bit(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x3 t = transpose(v_1(260u));
  float16_t l = length(tint_bitcast_to_16bit(v.inner[0u].z).yx);
  float16_t a = abs(tint_bitcast_to_16bit(v.inner[0u].z).yx.x);
}
