#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_0;
vec4 textureLoad_4acb64() {
  ivec2 v_1 = ivec2(ivec2(1));
  ivec3 v_2 = ivec3(v_1, int(1u));
  vec4 res = texelFetch(arg_0, v_2, int(1));
  return res;
}
void main() {
  v.inner = textureLoad_4acb64();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_0;
vec4 textureLoad_4acb64() {
  ivec2 v_1 = ivec2(ivec2(1));
  ivec3 v_2 = ivec3(v_1, int(1u));
  vec4 res = texelFetch(arg_0, v_2, int(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4acb64();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp sampler2DArray arg_0;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureLoad_4acb64() {
  ivec2 v = ivec2(ivec2(1));
  ivec3 v_1 = ivec3(v, int(1u));
  vec4 res = texelFetch(arg_0, v_1, int(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_4acb64();
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
