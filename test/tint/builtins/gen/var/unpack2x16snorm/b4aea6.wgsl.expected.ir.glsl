#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec2 inner;
} v;
vec2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  vec2 res = unpackSnorm2x16(arg_0);
  return res;
}
void main() {
  v.inner = unpack2x16snorm_b4aea6();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec2 inner;
} v;
vec2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  vec2 res = unpackSnorm2x16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = unpack2x16snorm_b4aea6();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

layout(location = 0) flat out vec2 vertex_main_loc0_Output;
vec2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  vec2 res = unpackSnorm2x16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec2(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = unpack2x16snorm_b4aea6();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
