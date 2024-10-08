#version 310 es
#extension GL_EXT_texture_shadow_lod: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v_1 = arg_2;
  uint v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v_3, float(v_2), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSampleLevel_36f0d3();
}
#version 310 es
#extension GL_EXT_texture_shadow_lod: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v_1 = arg_2;
  uint v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v_3, float(v_2), ivec2(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureSampleLevel_36f0d3();
}
#version 310 es
#extension GL_EXT_texture_shadow_lod: require


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DArrayShadow arg_0_arg_1;
layout(location = 0) flat out float vertex_main_loc0_Output;
float textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  vec2 v = arg_2;
  uint v_1 = arg_4;
  vec4 v_2 = vec4(v, float(arg_3), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v_2, float(v_1), ivec2(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0f);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleLevel_36f0d3();
  return tint_symbol;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = v_3.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
