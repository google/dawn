//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray f_arg_0;
vec4 textureLoad_4acb64() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  int arg_3 = 1;
  ivec2 v_2 = arg_1;
  uint v_3 = arg_2;
  int v_4 = arg_3;
  uint v_5 = min(v_3, (uint(textureSize(f_arg_0, 0).z) - 1u));
  uint v_6 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_7 = min(uint(v_4), v_6);
  uvec2 v_8 = (uvec2(textureSize(f_arg_0, int(v_7)).xy) - uvec2(1u));
  ivec2 v_9 = ivec2(min(uvec2(v_2), v_8));
  ivec3 v_10 = ivec3(v_9, int(v_5));
  vec4 res = texelFetch(f_arg_0, v_10, int(v_7));
  return res;
}
void main() {
  v.inner = textureLoad_4acb64();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray arg_0;
vec4 textureLoad_4acb64() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  int arg_3 = 1;
  ivec2 v_2 = arg_1;
  uint v_3 = arg_2;
  int v_4 = arg_3;
  uint v_5 = min(v_3, (uint(textureSize(arg_0, 0).z) - 1u));
  uint v_6 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_7 = min(uint(v_4), v_6);
  uvec2 v_8 = (uvec2(textureSize(arg_0, int(v_7)).xy) - uvec2(1u));
  ivec2 v_9 = ivec2(min(uvec2(v_2), v_8));
  ivec3 v_10 = ivec3(v_9, int(v_5));
  vec4 res = texelFetch(arg_0, v_10, int(v_7));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4acb64();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2DArray v_arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_4acb64() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  int arg_3 = 1;
  ivec2 v_1 = arg_1;
  uint v_2 = arg_2;
  int v_3 = arg_3;
  uint v_4 = min(v_2, (uint(textureSize(v_arg_0, 0).z) - 1u));
  uint v_5 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_6 = min(uint(v_3), v_5);
  uvec2 v_7 = (uvec2(textureSize(v_arg_0, int(v_6)).xy) - uvec2(1u));
  ivec2 v_8 = ivec2(min(uvec2(v_1), v_7));
  ivec3 v_9 = ivec3(v_8, int(v_4));
  vec4 res = texelFetch(v_arg_0, v_9, int(v_6));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_10 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_10.pos = vec4(0.0f);
  v_10.prevent_dce = textureLoad_4acb64();
  return v_10;
}
void main() {
  VertexOutput v_11 = vertex_main_inner();
  gl_Position = vec4(v_11.pos.x, -(v_11.pos.y), ((2.0f * v_11.pos.z) - v_11.pos.w), v_11.pos.w);
  tint_interstage_location0 = v_11.prevent_dce;
  gl_PointSize = 1.0f;
}
