#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec3 inner;
} v;
ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 v_1 = arg_0;
  uint v_2 = arg_2;
  uint v_3 = min(arg_1, 32u);
  uint v_4 = min(v_2, (32u - v_3));
  int v_5 = int(v_3);
  ivec3 res = bitfieldExtract(v_1, v_5, int(v_4));
  return res;
}
void main() {
  v.inner = extractBits_e04f5d();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec3 inner;
} v;
ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 v_1 = arg_0;
  uint v_2 = arg_2;
  uint v_3 = min(arg_1, 32u);
  uint v_4 = min(v_2, (32u - v_3));
  int v_5 = int(v_3);
  ivec3 res = bitfieldExtract(v_1, v_5, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = extractBits_e04f5d();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 v = arg_0;
  uint v_1 = arg_2;
  uint v_2 = min(arg_1, 32u);
  uint v_3 = min(v_1, (32u - v_2));
  int v_4 = int(v_2);
  ivec3 res = bitfieldExtract(v, v_4, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = extractBits_e04f5d();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
