#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_f16(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_7 = tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_8 = v.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_3, v_5, v_7, tint_bitcast_to_f16(mix(v_8.xy, v_8.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4 t = transpose(v_1(0u));
  float16_t l = length(tint_bitcast_to_f16(v.inner[0u].zw));
  float16_t a = abs(tint_bitcast_to_f16(v.inner[0u].xy).ywxz.x);
}
