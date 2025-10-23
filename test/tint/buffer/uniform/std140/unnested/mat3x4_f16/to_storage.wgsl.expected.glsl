#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat3x4 inner;
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_6 = tint_bitcast_to_f16_1(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_7 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3x4(v_4, v_6, tint_bitcast_to_f16_1(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = v_2(0u);
  v_1.inner[1u] = tint_bitcast_to_f16_1(v.inner[0u].xy);
  v_1.inner[1u] = tint_bitcast_to_f16_1(v.inner[0u].xy).ywxz;
  uvec4 v_8 = v.inner[0u];
  v_1.inner[0u].y = tint_bitcast_to_f16(v_8.z).x;
}
