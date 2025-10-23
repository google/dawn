#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
shared f16mat2x4 w;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_f16_1(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x4(v_3, tint_bitcast_to_f16_1(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
  }
  barrier();
  w = v_1(0u);
  w[1u] = tint_bitcast_to_f16_1(v.inner[0u].xy);
  w[1u] = tint_bitcast_to_f16_1(v.inner[0u].xy).ywxz;
  uvec4 v_5 = v.inner[0u];
  w[0u].y = tint_bitcast_to_f16(v_5.z).x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
