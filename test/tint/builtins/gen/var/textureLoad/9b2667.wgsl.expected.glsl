//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray f_arg_0;
float textureLoad_9b2667() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int arg_3 = 1;
  ivec2 v_2 = arg_1;
  int v_3 = arg_2;
  int v_4 = arg_3;
  uint v_5 = (uint(textureSize(f_arg_0, 0).z) - 1u);
  uint v_6 = min(uint(v_3), v_5);
  uint v_7 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_8 = min(uint(v_4), v_7);
  uvec2 v_9 = (uvec2(textureSize(f_arg_0, int(v_8)).xy) - uvec2(1u));
  ivec2 v_10 = ivec2(min(uvec2(v_2), v_9));
  ivec3 v_11 = ivec3(v_10, int(v_6));
  float res = texelFetch(f_arg_0, v_11, int(v_8)).x;
  return res;
}
void main() {
  v.inner = textureLoad_9b2667();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray arg_0;
float textureLoad_9b2667() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int arg_3 = 1;
  ivec2 v_2 = arg_1;
  int v_3 = arg_2;
  int v_4 = arg_3;
  uint v_5 = (uint(textureSize(arg_0, 0).z) - 1u);
  uint v_6 = min(uint(v_3), v_5);
  uint v_7 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_8 = min(uint(v_4), v_7);
  uvec2 v_9 = (uvec2(textureSize(arg_0, int(v_8)).xy) - uvec2(1u));
  ivec2 v_10 = ivec2(min(uvec2(v_2), v_9));
  ivec3 v_11 = ivec3(v_10, int(v_6));
  float res = texelFetch(arg_0, v_11, int(v_8)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_9b2667();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2DArray v_arg_0;
layout(location = 0) flat out float tint_interstage_location0;
float textureLoad_9b2667() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int arg_3 = 1;
  ivec2 v_1 = arg_1;
  int v_2 = arg_2;
  int v_3 = arg_3;
  uint v_4 = (uint(textureSize(v_arg_0, 0).z) - 1u);
  uint v_5 = min(uint(v_2), v_4);
  uint v_6 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_7 = min(uint(v_3), v_6);
  uvec2 v_8 = (uvec2(textureSize(v_arg_0, int(v_7)).xy) - uvec2(1u));
  ivec2 v_9 = ivec2(min(uvec2(v_1), v_8));
  ivec3 v_10 = ivec3(v_9, int(v_5));
  float res = texelFetch(v_arg_0, v_10, int(v_7)).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_11 = VertexOutput(vec4(0.0f), 0.0f);
  v_11.pos = vec4(0.0f);
  v_11.prevent_dce = textureLoad_9b2667();
  return v_11;
}
void main() {
  VertexOutput v_12 = vertex_main_inner();
  gl_Position = vec4(v_12.pos.x, -(v_12.pos.y), ((2.0f * v_12.pos.z) - v_12.pos.w), v_12.pos.w);
  tint_interstage_location0 = v_12.prevent_dce;
  gl_PointSize = 1.0f;
}
