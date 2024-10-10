#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
ivec4 bitcast_3f7437() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = arg_0;
  return res;
}
void main() {
  v.inner = bitcast_3f7437();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
ivec4 bitcast_3f7437() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = arg_0;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = bitcast_3f7437();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 vertex_main_loc0_Output;
ivec4 bitcast_3f7437() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = arg_0;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec4(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_3f7437();
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
