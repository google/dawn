#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xu8_clamp(uvec4 a) {
  uvec4 a_clamp = clamp(a, uvec4(0u), uvec4(255u));
  uvec4 a_u8 = uvec4((a_clamp << uvec4(0u, 8u, 16u, 24u)));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uint res = tint_pack_4xu8_clamp(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  pack4xU8Clamp_6b8c1b();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xu8_clamp(uvec4 a) {
  uvec4 a_clamp = clamp(a, uvec4(0u), uvec4(255u));
  uvec4 a_u8 = uvec4((a_clamp << uvec4(0u, 8u, 16u, 24u)));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uint res = tint_pack_4xu8_clamp(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  pack4xU8Clamp_6b8c1b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xu8_clamp(uvec4 a) {
  uvec4 a_clamp = clamp(a, uvec4(0u), uvec4(255u));
  uvec4 a_u8 = uvec4((a_clamp << uvec4(0u, 8u, 16u, 24u)));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uint res = tint_pack_4xu8_clamp(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  pack4xU8Clamp_6b8c1b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
