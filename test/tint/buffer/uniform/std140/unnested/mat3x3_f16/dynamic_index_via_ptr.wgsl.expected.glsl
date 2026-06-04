#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[2];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec3 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))).xyz;
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  return f16mat3(v_4, v_7, tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))).xyz);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (min(uint(i()), 2u) * 8u);
  f16mat3 l_m = v_2(0u);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  f16vec3 l_m_i = tint_bitcast_to_16bit(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))).xyz;
}
