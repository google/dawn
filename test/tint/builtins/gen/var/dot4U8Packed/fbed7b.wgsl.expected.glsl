#version 310 es
precision highp float;
precision highp int;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_dot4_u8_packed(uint a, uint b) {
  uvec4 a_u8 = ((uvec4(a) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  uvec4 b_u8 = ((uvec4(b) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  return tint_int_dot(a_u8, b_u8);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = tint_dot4_u8_packed(arg_0, arg_1);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = dot4U8Packed_fbed7b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

uint tint_dot4_u8_packed(uint a, uint b) {
  uvec4 a_u8 = ((uvec4(a) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  uvec4 b_u8 = ((uvec4(b) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  return tint_int_dot(a_u8, b_u8);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = tint_dot4_u8_packed(arg_0, arg_1);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void compute_main() {
  prevent_dce.inner = dot4U8Packed_fbed7b();
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

uint tint_dot4_u8_packed(uint a, uint b) {
  uvec4 a_u8 = ((uvec4(a) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  uvec4 b_u8 = ((uvec4(b) >> uvec4(24u, 16u, 8u, 0u)) & uvec4(255u));
  return tint_int_dot(a_u8, b_u8);
}

layout(location = 0) flat out uint prevent_dce_1;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = tint_dot4_u8_packed(arg_0, arg_1);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
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
