#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
shared f16mat2 w;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_2, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = f16mat2(f16vec2(0.0hf), f16vec2(0.0hf));
  }
  barrier();
  w = v_1(0u);
  w[1u] = tint_bitcast_to_f16(v.inner[0u].x);
  w[1u] = tint_bitcast_to_f16(v.inner[0u].x).yx;
  uvec4 v_3 = v.inner[0u];
  w[0u].y = tint_bitcast_to_f16(v_3.y).x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
