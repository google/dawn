#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_data_block_ubo {
  uvec4 inner[2];
} v;
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16_1(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  f16vec2 v_3 = tint_bitcast_to_f16_1(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return f16mat3x2(v_2, v_3, tint_bitcast_to_f16_1(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}
void main() {
  f16mat3x2 v_4 = v_1(0u);
  f16vec2 x = (v_4 * tint_bitcast_to_f16(v.inner[1u].xy).xyz);
}
