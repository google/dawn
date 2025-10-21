#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat3 t = transpose(v_1(264u));
  float16_t l = length(tint_bitcast_to_f16(v.inner[1u].xy).xyz.zxy);
  float16_t a = abs(tint_bitcast_to_f16(v.inner[1u].xy).xyz.zxy.x);
}
