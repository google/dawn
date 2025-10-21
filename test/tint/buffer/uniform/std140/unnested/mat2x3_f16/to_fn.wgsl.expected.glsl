#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v_1;
void a(f16mat2x3 m) {
}
void b(f16vec3 v) {
}
void c(float16_t f_1) {
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16mat2x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u)))).xyz;
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_4, tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))).xyz);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(v_1.inner[0u].zw).xyz);
  b(tint_bitcast_to_f16(v_1.inner[0u].zw).xyz.zxy);
  uvec4 v_6 = v_1.inner[0u];
  c(tint_bitcast_to_f16_1(v_6.z).x);
  c(tint_bitcast_to_f16(v_1.inner[0u].zw).xyz.zxy.x);
}
