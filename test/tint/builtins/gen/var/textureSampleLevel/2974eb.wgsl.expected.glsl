//
// fragment_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSampleLevel_2974eb() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v_1 = arg_2;
  uint v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3), 0.0f);
  float res = textureLod(f_arg_0_arg_1, v_3, float(v_2));
  return res;
}
void main() {
  v.inner = textureSampleLevel_2974eb();
}
//
// compute_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleLevel_2974eb() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v_1 = arg_2;
  uint v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3), 0.0f);
  float res = textureLod(arg_0_arg_1, v_3, float(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureSampleLevel_2974eb();
}
//
// vertex_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DArrayShadow v_arg_0_arg_1;
layout(location = 0) flat out float tint_interstage_location0;
float textureSampleLevel_2974eb() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v = arg_2;
  uint v_1 = arg_4;
  vec4 v_2 = vec4(v, float(arg_3), 0.0f);
  float res = textureLod(v_arg_0_arg_1, v_2, float(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), 0.0f);
  v_3.pos = vec4(0.0f);
  v_3.prevent_dce = textureSampleLevel_2974eb();
  return v_3;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.pos.x, -(v_4.pos.y), ((2.0f * v_4.pos.z) - v_4.pos.w), v_4.pos.w);
  tint_interstage_location0 = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
