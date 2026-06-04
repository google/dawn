#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v_1;
void a(f16mat4x3 m) {
}
void b(f16vec3 v) {
}
void c(float16_t f_1) {
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16vec2 tint_bitcast_to_16bit_1(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  f16vec3 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))).xyz;
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v_1.inner[(v_8 / 16u)];
  f16vec3 v_10 = tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))).xyz;
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v_1.inner[(v_11 / 16u)];
  return f16mat4x3(v_4, v_7, v_10, tint_bitcast_to_16bit(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))).xyz);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_2(0u));
  b(tint_bitcast_to_16bit(v_1.inner[0u].zw).xyz);
  b(tint_bitcast_to_16bit(v_1.inner[0u].zw).xyz.zxy);
  uvec4 v_13 = v_1.inner[0u];
  c(tint_bitcast_to_16bit_1(v_13.z).x);
  c(tint_bitcast_to_16bit(v_1.inner[0u].zw).xyz.zxy.x);
}
