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
f16mat3 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_3 = tint_bitcast_to_16bit(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  f16vec3 v_6 = tint_bitcast_to_16bit(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u)))).xyz;
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return f16mat3(v_3, v_6, tint_bitcast_to_16bit(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u)))).xyz);
}
void main() {
  f16mat3 v_9 = v_1(0u);
  f16vec3 x = (v_9 * tint_bitcast_to_16bit(v.inner[1u].zw).xyz);
}
