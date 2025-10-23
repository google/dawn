#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v_1;
void a(f16mat4x2 m) {
}
void b(f16vec2 v) {
}
void c(float16_t f_1) {
}
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v_1.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v_1.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  f16vec2 v_5 = tint_bitcast_to_f16(v_1.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]);
  return f16mat4x2(v_3, v_4, v_5, tint_bitcast_to_f16(v_1.inner[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(v_1.inner[0u].y));
  b(tint_bitcast_to_f16(v_1.inner[0u].y).yx);
  uvec4 v_6 = v_1.inner[0u];
  c(tint_bitcast_to_f16(v_6.y).x);
  c(tint_bitcast_to_f16(v_1.inner[0u].y).yx.x);
}
