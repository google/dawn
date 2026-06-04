#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_data_block_ubo {
  uvec4 inner[2];
} v;
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16vec2 tint_bitcast_to_16bit_1(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit_1(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit_1(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  return f16mat3x2(v_2, v_4, tint_bitcast_to_16bit_1(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}
void main() {
  f16mat3x2 v_6 = v_1(0u);
  f16vec2 x = (v_6 * tint_bitcast_to_16bit(v.inner[1u].xy).xyz);
}
