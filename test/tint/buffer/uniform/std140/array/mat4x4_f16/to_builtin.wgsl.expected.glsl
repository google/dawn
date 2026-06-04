#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec4 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  f16vec4 v_10 = tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u))));
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v.inner[(v_11 / 16u)];
  return f16mat4(v_4, v_7, v_10, tint_bitcast_to_16bit(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4 t = transpose(v_2(64u));
  float16_t l = length(tint_bitcast_to_16bit(v.inner[0u].zw).ywxz);
  float16_t a = abs(tint_bitcast_to_16bit(v.inner[0u].zw).ywxz.x);
  float16_t v_13 = (t[0u].x + float16_t(l));
  v_1.inner = (v_13 + float16_t(a));
}
