#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uniform highp usampler2DMS arg_0;
uvec4 textureLoad_c378ee() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_1 = arg_1;
  int v_2 = arg_2;
  uvec2 v_3 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(v_1), v_3));
  uvec4 res = texelFetch(arg_0, v_4, int(v_2));
  return res;
}
void main() {
  v.inner = textureLoad_c378ee();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uniform highp usampler2DMS arg_0;
uvec4 textureLoad_c378ee() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_1 = arg_1;
  int v_2 = arg_2;
  uvec2 v_3 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(v_1), v_3));
  uvec4 res = texelFetch(arg_0, v_4, int(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c378ee();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

uniform highp usampler2DMS arg_0;
layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 textureLoad_c378ee() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v = arg_1;
  int v_1 = arg_2;
  uvec2 v_2 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(v), v_2));
  uvec4 res = texelFetch(arg_0, v_3, int(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_c378ee();
  return tint_symbol;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = v_4.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
