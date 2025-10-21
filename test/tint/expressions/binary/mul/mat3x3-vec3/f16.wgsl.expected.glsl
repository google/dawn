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
f16mat3 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_3 = tint_bitcast_to_f16(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u)))).xyz;
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec3 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))).xyz;
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3(v_3, v_5, tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))).xyz);
}
void main() {
  f16mat3 v_7 = v_1(0u);
  f16vec3 x = (v_7 * tint_bitcast_to_f16(v.inner[1u].zw).xyz);
}
