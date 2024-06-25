#version 310 es
precision highp float;
precision highp int;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_u32 = uvec4(a);
  uvec4 a_u8 = ((a_u32 & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = pack4xI8_bfce01();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_u32 = uvec4(a);
  uvec4 a_u8 = ((a_u32 & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void compute_main() {
  prevent_dce.inner = pack4xI8_bfce01();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_u32 = uvec4(a);
  uvec4 a_u8 = ((a_u32 & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u));
  return tint_int_dot(a_u8, uvec4(1u));
}

layout(location = 0) flat out uint prevent_dce_1;
uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xI8_bfce01();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
