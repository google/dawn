#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DMS arg_0;
float textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uint v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  float res = texelFetch(arg_0, v_2, int(v_1)).x;
  return res;
}
void main() {
  v.inner = textureLoad_4db25c();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DMS arg_0;
float textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uint v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  float res = texelFetch(arg_0, v_2, int(v_1)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4db25c();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DMS arg_0;
layout(location = 0) flat out float vertex_main_loc0_Output;
float textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uint v = arg_2;
  ivec2 v_1 = ivec2(arg_1);
  float res = texelFetch(arg_0, v_1, int(v)).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0f);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_4db25c();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
